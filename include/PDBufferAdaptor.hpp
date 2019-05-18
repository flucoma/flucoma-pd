#pragma once

#include <clients/common/BufferAdaptor.hpp>
#include <data/FluidTensor.hpp>
#include <atomic>

#include "m_pd.h"

namespace fluid {
namespace client {
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
    //FIX
    return false;
  }

  bool valid() const override
  {
    //FIX
    return false;
  }

  void resize(size_t frames, size_t channels, size_t rank, double sampleRate) override
  {
    //FIX

    /*
      assert(frames == numFrames() && channels == numChans());
    }*/
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
    //FIX
    float* samples = nullptr;
    FluidTensorView<float, 1> v{samples, 0, numFrames()};
    return v;
  }

  FluidTensorView<float, 1> samps(size_t offset, size_t nframes, size_t chanoffset) override
  {
    //FIX
    float* samples = nullptr;
    FluidTensorView<float, 1> v{samples, 0, numFrames()};
    
    return v(Slice(offset, nframes));
  }
    
  size_t numFrames() const override { return valid() ? getMaxFrames() : 0u; }

  size_t numChans() const override { return valid() ? getChannelCount() / mRank : 0u; }

  size_t rank() const override { return valid() ? mRank : 0; }
  
  //FIX
  double sampleRate() const override { return valid() ? 1 : 0 ; } // N.B. pd has no notion of sample rates for buffers...

private:

  size_t getMaxFrames() const
  {
    //FIX
    return 1;
  }
    
  size_t getChannelCount() const
  {
    //FIX
    return 1;
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

    mName   = other.mName;
    mRank   = other.mRank;

    releaseLock();
  }

  t_symbol *mName;
  size_t   mRank;
    
  std::atomic<bool> mLock;
};
} // namespace client
} // namespace fluid

