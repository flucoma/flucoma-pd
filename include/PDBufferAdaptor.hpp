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
    return channelExists(0, 0);
  }

  bool valid() const override
  {
    return getMinFrames() > 0;
  }

  void resize(size_t frames, size_t channels, size_t rank, double sampleRate) override
  {
    NOTUSED(sampleRate);
      
    size_t nChans = numChans();
      
    for (size_t i = 0; i < nChans && i < channels; i++)
    {
      for (size_t j = 0; j < mRank && j < rank; j++)
      {
        t_garray *array = getArray(channelName(i, j));
          
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
    float* samples = (float *) getChannelSamples(channel, rankIdx);
      
    FluidTensorView<float, 1> v{samples, 0, numFrames(), sizeof(t_word) / sizeof(float)};
    return v;
  }

  FluidTensorView<float, 1> samps(size_t offset, size_t nframes, size_t chanoffset) override
  {
    float* samples = (float *) getChannelSamples(chanoffset, 0);
    FluidTensorView<float, 1> v{samples, 0, numFrames(), sizeof(t_word) / sizeof(float)};
    
    return v(Slice(offset, nframes));
  }
    
  size_t numFrames() const override { return valid() ? getMinFrames() : 0u; }

  size_t numChans() const override { return valid() ? getChannelCount() / mRank : 0u; }

  size_t rank() const override { return valid() ? mRank : 0; }
  
  //FIX
  double sampleRate() const override { return valid() ? 1 : 0 ; } // N.B. pd has no notion of sample rates for buffers...

private:
    
  size_t getMinFrames() const
  {
    size_t nChans = numChans();
    size_t frames = 0;
    
    for (size_t i = 0; i < nChans; i++)
    {
      for (size_t j = 0; j < mRank; j++)
      {
        size_t chanFrames = getChannelFrames(i, j);
        frames = (!i && !j) ? chanFrames : std::min(chanFrames, frames);
      }
    }
    
    return frames;
  }
    
  size_t getChannelCount() const
  {
    //FIX
    return 1;
  }
    
  t_symbol *channelName(size_t chan, size_t rank) const
  {
    NOTUSED(chan);
    NOTUSED(rank);
      
    //FIX
    return mName;
  }
    
  size_t getChannelFrames(size_t chan, size_t rank) const
  {
    t_word *words;
    int length = 0;
    getChannel(channelName(chan, rank), words, length);
    return static_cast<size_t>(length);
  }
    
  t_word *getChannelSamples(size_t chan, size_t rank)
  {
    t_word *words;
    int length;
    getChannel(channelName(chan, rank), words, length);
    return words;
  }
    
  bool channelExists(size_t chan, size_t rank) const
  {
    return getArray(channelName(chan, rank));
  }
    
  void getChannel(t_symbol *name, t_word*& words, int& length) const
  {
    t_garray *array = getArray(name);
    
    if (array)
    {
      garray_getfloatwords(array, &length, &words);
    }
    else
    {
      words = nullptr;
      length = 0;
    }
  }
    
  t_garray *getArray(t_symbol *name) const
  {
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

