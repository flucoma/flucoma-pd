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
  PDBufferAdaptor(t_object *x, t_symbol *name)
      : mHostObject(x)
      , mName(name)
      , mSamps(nullptr)
      //, mBufref{buffer_ref_new(mHostObject, mName)}
      , mRank(1)
      , mLock(false)
  {}

  ~PDBufferAdaptor()
  {
    while (!tryLock());

    release();
    //if (mBufref) object_free(mBufref);
  }

  PDBufferAdaptor(const PDBufferAdaptor &) = delete;
  PDBufferAdaptor &operator=(const PDBufferAdaptor &other) = delete;

  PDBufferAdaptor(PDBufferAdaptor &&other) //: MaxBufferView(other.mHostObject, other.mName)
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
    // return getBuffer(); <--Doesn't work on 0-size buffers
      return false;//buffer_ref_exists(mBufref);
  }

  bool valid() const override
  {
    return mSamps;
    //      return getBuffer();
  }

  void resize(size_t frames, size_t channels, size_t rank,double sampleRate) override
  {
    /*t_object *buffer = getBuffer();

    if (buffer)
    {
      // Do this in two stages so we can set length in samps rather than ms
      unlockSamps();
      buffer_edit_begin(buffer);

      t_atom args[2];
      atom_setfloat(&args[0], 0.);
      atom_setlong(&args[1], static_cast<t_atom_long>(rank * channels));
      t_symbol *setSizeMsg = gensym("setsize");
      object_method_typed(buffer, setSizeMsg, 2, args, nullptr);
      object_method(buffer, gensym("dirty"));
      t_atom newsize;
      atom_setlong(&newsize, static_cast<t_atom_long>(frames));
      t_symbol *sampsMsg = gensym("sizeinsamps");
      object_method_typed(buffer, sampsMsg, 1, &newsize, nullptr);
      t_atom sr;
      atom_setfloat(&sr,sampleRate);
      t_symbol *srMsg = gensym("sr");
      object_method_typed(buffer,srMsg,1,&sr,nullptr); 
      object_method(buffer, gensym("dirty"));
      buffer_edit_end(buffer, 1);
      lockSamps();
      mRank = rank;
      assert(frames == numFrames() && channels == numChans());
    }*/
  }

  bool acquire() override
  {
    bool lock = tryLock();
      
    if (lock)
    {
      //lockSamps();
      return true;
    }
      
    return false;
  }

  void release() override
  {
    //unlockSamps();
    releaseLock();
  }

  FluidTensorView<float, 1> samps(size_t channel, size_t rankIdx = 0) override
  {
    FluidTensorView<float, 2> v{this->mSamps, 0, numFrames(), numChans() * this->mRank};

    return v.col(rankIdx + channel * mRank);
  }

  FluidTensorView<float, 1> samps(size_t offset, size_t nframes, size_t chanoffset) override
  {
    FluidTensorView<float, 2> v{this->mSamps, 0, numFrames(), numChans() * this->mRank};
    return v(Slice(offset, nframes), Slice(chanoffset, 1)).col(0);
  }
    
  size_t numFrames() const override { return 0; }//valid() ? static_cast<size_t>(buffer_getframecount(getBuffer())) : 0; }

  size_t numChans() const override { return 0; }//valid() ? static_cast<size_t>(buffer_getchannelcount(getBuffer())) / mRank : 0; }

  size_t rank() const override { return valid() ? mRank : 0; }
  
    double sampleRate() const override { return 44100; } //valid() ? buffer_getsamplerate(getBuffer()) : 0; }

private:

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
    //object_free(mBufref);

    mSamps  = other.mSamps;
    //mBufref = other.mBufref;
    mRank   = other.mRank;

    other.mSamps  = nullptr;
    //other.mBufref = nullptr;
    releaseLock();
  }

  t_object *mHostObject;
  t_symbol *mName;

  float *       mSamps;
  size_t        mRank;
  std::atomic<bool> mLock;
};
} // namespace client
} // namespace fluid

