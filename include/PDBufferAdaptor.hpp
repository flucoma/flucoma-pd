#pragma once

#include <clients/common/BufferAdaptor.hpp>
#include <clients/common/Result.hpp>
#include <data/FluidTensor.hpp>
#include <atomic>

#include "m_pd.h"

namespace fluid {
namespace client {
    
template<typename T>
void NOTUSED(T& a)
{
  (void)sizeof(a);
}
    
class PDBufferAdaptor : public BufferAdaptor
{
public:
  PDBufferAdaptor(t_symbol *name, double sr)
      : mName(name)
      , mLock(false)
      , mSampleRate(sr)
  {}

  ~PDBufferAdaptor()
  {
    while (!tryLock());

    release();
  }

  PDBufferAdaptor(const PDBufferAdaptor &) = delete;
  PDBufferAdaptor &operator=(const PDBufferAdaptor &other) = delete;

  PDBufferAdaptor(PDBufferAdaptor &&other)
  {
    swap(std::move(other));
  }

  PDBufferAdaptor &operator=(PDBufferAdaptor &&other)
  {
    swap(std::move(other));
    return *this;
  }

  t_symbol *name() const { return mName; }

  bool exists() const override
  {
    return numChans();
  }

  bool valid() const override
  {
    return getMinFrames() > 0;
  }

  const Result resize(size_t frames, size_t channels, double /*sampleRate*/) override
  {
//    NOTUSED(sampleRate);
    
    size_t nChans = numChans();
    Result r;
    if(channels > numChans())
    {
      r.set(Result::Status::kError);
      r.addMessage("Not enough arrays to operate on ", mName->s_name, ". Found ", channels, " array(s) but need ", nChans, ".");
    }
    
    for (size_t i = 0; i < nChans; ++i)
    {
      t_garray *array = getArray(i);
          
      if (array)
        garray_resize_long(array, static_cast<int>(frames));
    }
      
    if(frames != numFrames())
    {
      r.set(Result::Status::kError);
      r.addMessage("Reszie failed on ", mName->s_name, ".");
    }
    return r;
  }

  bool acquire() const override
  {
    bool lock = tryLock();
      
    if (lock)
    {
      return true;
    }
      
    return false;
  }

  void release() const override
  {
    releaseLock();
  }

  FluidTensorView<float, 1> samps(size_t channel) override
  {
    float* samples = (float *) getArrayData(channel);
      
    FluidTensorView<float, 2> v{samples, 0, numFrames(), sizeof(t_word) / sizeof(float)};
      
    return v.col(0);
  }

  FluidTensorView<float, 1> samps(size_t offset, size_t nframes, size_t chanoffset) override
  {
    float* samples = (float *) getArrayData(chanoffset);
    FluidTensorView<float, 2> v{samples, 0, numFrames(), sizeof(t_word) / sizeof(float)};
    
    return v(Slice(offset, nframes), Slice(0, 1)).col(0);
  }
  
  FluidTensorView<const float, 1> samps(size_t channel) const override
  {
    float* samples = (float *) getArrayData(channel);
    
    FluidTensorView<const float, 2> v{samples, 0, numFrames(), sizeof(t_word) / sizeof(float)};
    
    return v.col(0);
  }

  FluidTensorView<const float, 1> samps(size_t offset, size_t nframes, size_t chanoffset) const override
  {
    float* samples = (float *) getArrayData(chanoffset);
    FluidTensorView<const float, 2> v{samples, 0, numFrames(), sizeof(t_word) / sizeof(float)};
    
    return v(Slice(offset, nframes), Slice(0, 1)).col(0);
  }
    
  size_t numFrames() const override { return getMinFrames(); }

  size_t numChans() const override { return getArrayCount(); }
  
  double sampleRate() const override { return mSampleRate; } // N.B. pd has no notion of sample rates for buffers...
  void sampleRate(double sr) const { mSampleRate = sr; } //we still need to set the SR on const input buffers
  std::string asString() const override { return mName->s_name; }
private:
    
  size_t getMinFrames() const
  {
    size_t nChans = numChans();
    int frames = 0;
    
    for (size_t i = 0; i < nChans; i++)
    {
      int arrayFrames = 0;
      getArrayData(i, &arrayFrames);
      frames = !i ? arrayFrames : std::min(arrayFrames, frames);
    }
    
    return static_cast<size_t>(frames);
  }
    
  size_t getArrayCount() const
  {
    size_t count = 0;
    
    for (size_t i = 0; ; i++, count++)
        if (!getArray(i))
          break;
      
    return count;
  }
    
  t_word *getArrayData(size_t chan, int* retLength = nullptr) const
  {
    t_garray *array = getArray(chan);
    t_word *words = nullptr;
    int length = 0;
    
    if (array)
      garray_getfloatwords(array, &length, &words);
    
    if (retLength)
      *retLength = length;
    
    return words;
  }
    
  t_garray *getArray(size_t chan) const
  {
    t_symbol *name = mName;
      
    if (chan || !pd_findbyclass(name, garray_class))
    {
      char nameString[MAXPDSTRING];
      int number = static_cast<int>(chan);
        
      snprintf(nameString, MAXPDSTRING, "%s-%d", mName->s_name, number);
    
      name = gensym(nameString);
    }

    return (t_garray *) pd_findbyclass(name, garray_class);
  }
    
  bool tryLock() const
  {
    return compareExchange(false, true);
  }
  
  void releaseLock() const
  {
    compareExchange(true, false);
  }
    
  bool compareExchange(bool compare, bool exchange) const
  {
    return mLock.compare_exchange_strong(compare, exchange);
  }
    
  void swap(PDBufferAdaptor &&other)
  {
    while (!tryLock());
      
    release();

    mName = other.mName;

    releaseLock();
  }

  t_symbol *mName;
    
  mutable std::atomic<bool> mLock;
  mutable double mSampleRate;
};
} // namespace client
} // namespace fluid

