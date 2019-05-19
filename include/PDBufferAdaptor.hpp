#pragma once

#include <clients/common/BufferAdaptor.hpp>
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
  PDBufferAdaptor(t_symbol *name)
      : mName(name)
      , mRank(1)
      , mLock(false)
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

  t_symbol * name() const { return mName; }

  bool exists() const override
  {
    return numChans();
  }

  bool valid() const override
  {
    return getMinFrames() > 0;
  }

  void resize(size_t frames, size_t channels, size_t rank, double sampleRate) override
  {
    NOTUSED(sampleRate);
      
    size_t nChans = numChans();
    mRank = rank;
      
    for (size_t i = 0; i < nChans && i < channels; i++)
    {
      for (size_t j = 0; j < mRank && j < rank; j++)
      {
        t_garray *array = getArray(i, j);
          
        if (array)
          garray_resize_long(array, static_cast<int>(frames));
      }
    }
      
    assert(frames == numFrames() && channels == numChans());
  }

  bool acquire() override
  {
    bool lock = tryLock();
      
    if (lock)
    {
      return true;
    }
      
    return false;
  }

  void release() override
  {
    releaseLock();
  }

  FluidTensorView<float, 1> samps(size_t channel, size_t rankIdx = 0) override
  {
    float* samples = (float *) getArrayData(channel, rankIdx);
      
    FluidTensorView<float, 2> v{samples, 0, numFrames(), sizeof(t_word) / sizeof(float)};
      
    return v.col(0);
  }

  FluidTensorView<float, 1> samps(size_t offset, size_t nframes, size_t chanoffset) override
  {
    float* samples = (float *) getArrayData(chanoffset, 0);
    FluidTensorView<float, 2> v{samples, 0, numFrames(), sizeof(t_word) / sizeof(float)};
    
    return v(Slice(offset, nframes), Slice(0, 1)).col(0);
  }
    
  size_t numFrames() const override { return getMinFrames(); }

  size_t numChans() const override { return getArrayCount() / mRank; }

  size_t rank() const override { return valid() ? mRank : 0; }
  
  //FIX
  double sampleRate() const override { return valid() ? 1 : 0 ; } // N.B. pd has no notion of sample rates for buffers...

private:
    
  size_t getMinFrames() const
  {
    size_t nChans = numChans();
    int frames = 0;
    
    for (size_t i = 0; i < nChans; i++)
    {
      for (size_t j = 0; j < mRank; j++)
      {
        int arrayFrames = 0;
        
        getArrayData(i, j, &arrayFrames);
        frames = (!i && !j) ? arrayFrames : std::min(arrayFrames, frames);
      }
    }
    
    return static_cast<size_t>(frames);
  }
    
  size_t getArrayCount() const
  {
    size_t count = 0;
    
    for (size_t i = 0; ; i++)
      for (size_t j = 0; j < mRank; j++, count++)
        if (!getArray(i, j))
          return count;
      
    return count;
  }
    
  t_word *getArrayData(size_t chan, size_t rank, int* retLength = nullptr) const
  {
    t_garray *array = getArray(chan, rank);
    t_word *words = nullptr;
    int length = 0;
    
    if (array)
      garray_getfloatwords(array, &length, &words);
    
    if (retLength)
      *retLength = length;
    
    return words;
  }
    
  t_garray *getArray(size_t chan, size_t rank) const
  {
    t_symbol *name = mName;
      
    if (chan || rank || !pd_findbyclass(name, garray_class))
    {
      char nameString[MAXPDSTRING];
      int number = static_cast<int>(mRank * chan + rank) + 1;
        
      snprintf(nameString, MAXPDSTRING, "%s-%d", mName->s_name, number);
    
      name = gensym(nameString);
    }

    return (t_garray *) pd_findbyclass(name, garray_class);
  }
    
  bool tryLock()
  {
    return compareExchange(false, true);
  }
  
  void releaseLock()
  {
    compareExchange(true, false);
  }
    
  bool compareExchange(bool compare, bool exchange)
  {
    return mLock.compare_exchange_strong(compare, exchange);
  }
    
  void swap(PDBufferAdaptor &&other)
  {
    while (!tryLock());
      
    release();

    mName = other.mName;
    mRank = other.mRank;

    releaseLock();
  }

  t_symbol *mName;
  size_t   mRank;
    
  std::atomic<bool> mLock;
};
} // namespace client
} // namespace fluid

