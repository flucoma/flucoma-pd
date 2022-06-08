/*
Part of the Fluid Corpus Manipulation Project (http://www.flucoma.org/)
Copyright 2017-2019 University of Huddersfield.
Licensed under the BSD-3 License.
See license.md file in the project root for full license information.
This project has received funding from the European Research Council (ERC)
under the European Union’s Horizon 2020 research and innovation programme
(grant agreement No 725899).
*/

#pragma once

#include "PDBufferAdaptor.hpp"

#include <FluidVersion.hpp>

#include <clients/common/FluidBaseClient.hpp>
#include <clients/common/FluidNRTClientWrapper.hpp>
#include <clients/common/OfflineClient.hpp>
#include <clients/common/ParameterSet.hpp>
#include <clients/common/ParameterTypes.hpp>
#include <clients/nrt/FluidSharedInstanceAdaptor.hpp>

#include <m_pd.h>

#include <cctype>
#include <cstdarg>
#include <cstring>
#include <chrono>
#include <tuple>
#include <utility>
#include <map>

namespace fluid {
namespace client {


namespace dataset {
  class DataSetClient;
}

namespace labelset {
  class LabelSetClient;
}

template<typename T>
const char* SharedClientName="Unrecognised object";

template<>
const char* SharedClientName<dataset::DataSetClient> ="fluid.dataset";

template<>
const char* SharedClientName<labelset::LabelSetClient> ="fluid.labelset";

////////////////////////////////////////////////////////////////////////////////

namespace impl {

template <class T>
void object_warn(T* x, const char* fmt, ...)
{
  char postString[1024];

  const char* objectName = class_getname(*((t_pd*) x));
  va_list     argp;
  va_start(argp, fmt);
  vsnprintf(postString, 1024, fmt, argp);
  va_end(argp);

  post("%s: %s", objectName, postString);
}

////////////////////////////////////////////////////////////////////////////////

template <class Wrapper>
class RealTime
{
  using ViewType = FluidTensorView<t_sample, 1>;

public:
  // Make sure we free the clock if it's in use, otherwise badness
  ~RealTime()
  {
    if (mControlClock) clock_free(mControlClock);
  }

  static void setup(t_class* c)
  {
    class_addmethod(c, nullfn, gensym("signal"), A_NULL);
    class_addmethod(c, (t_method) callDSP, gensym("dsp"), A_CANT, 0);

    class_addmethod(c, (t_method) getLatency, gensym("getlatency"), A_NULL);
  }

  static void getLatency(Wrapper* x)
  {
    outlet_float(x->mLatencyOut, static_cast<t_float>(x->mClient.latency()));
  }

  static void callDSP(Wrapper* x, t_signal** sp) { x->dsp(sp); }

  static t_int* callPerform(t_int* w)
  {
    Wrapper* x = (Wrapper*) w[1];
    int      sampleframes = (int) w[2];
    x->perform(sampleframes);
    return w + 3;
  }

  void setupAudio(t_object* pdObject, index numSigIns, index numSigOuts)
  {

    mSigIns.resize(asUnsigned(numSigIns));
    mSigOuts.resize(asUnsigned(numSigOuts));

    // Create signal inlets

    for (index i = 0; numSigIns && i < (numSigIns - 1); i++)
      signalinlet_new(pdObject, 0.0);

    // Create signal outlets

    for (index i = 0; i < numSigOuts; i++)
      outlet_new(pdObject, gensym("signal"));
  }

  void dsp(t_signal** sp)
  {
    Wrapper* wrapper = static_cast<Wrapper*>(this);
    static_assert(sizeof(PD_LONGINTTYPE) == sizeof(intptr_t),
                  "size of pointer int type is wrongwrongwrong");

    if (!Wrapper::template IsModel_t<typename Wrapper::ClientType>::value)
      wrapper->mClient = typename Wrapper::ClientType{wrapper->mParams};


    auto& client = wrapper->client();
    client.sampleRate(sp[0]->s_sr);

    assert((client.audioChannelsOut() > 0) !=
               (client.controlChannelsOut().count > 0) &&
           "Client must *either* be audio out or control out, sorry");

    mInputs = std::vector<ViewType>(asUnsigned(client.audioChannelsIn()),
                                    ViewType(nullptr, 0, 0));

    if (client.audioChannelsOut() > 0)
      mOutputs = std::vector<ViewType>(asUnsigned(client.audioChannelsOut()),
                                       ViewType(nullptr, 0, 0));

    if (client.controlChannelsOut().count > 0 && client.audioChannelsIn() > 0)
    {
      mControlClock = mControlClock ? mControlClock
                                    : clock_new((t_object*) wrapper,
                                                (t_method) doControlOut);
      mControlOutputs.resize(asUnsigned(client.maxControlChannelsOut()));
      mControlAtoms.resize(asUnsigned(client.maxControlChannelsOut()));
      mOutputs.emplace_back(mControlOutputs.data(), 0, mControlOutputs.size()); 
    }

    for (index i = 0; i < asSigned(mSigIns.size()); i++)
      mSigIns[asUnsigned(i)] = sp[i]->s_vec;

    for (index i = 0; i < asSigned(mSigOuts.size()); i++)
      mSigOuts[asUnsigned(i)] = sp[i + asSigned(mSigIns.size())]->s_vec;


    if (!(client.controlChannelsIn() > 0))
      dsp_add(callPerform, 2, wrapper, sp[0]->s_vecsize);
  }

  void perform(int sampleframes)
  {
    auto& client = static_cast<Wrapper*>(this)->mClient;

    for (size_t i = 0, numins = mSigIns.size(); i < numins; ++i)
      mInputs[i].reset(mSigIns[i], 0, sampleframes);

    if (client.audioChannelsOut())
      for (size_t i = 0, numouts = asUnsigned(client.audioChannelsOut());
           i < numouts; ++i)
        mOutputs[i].reset(mSigOuts[i], 0, sampleframes);

    client.process(mInputs, mOutputs, mContext);

    if (mControlClock) clock_delay(mControlClock, 0);
  }

  void controlData()
  {
    Wrapper* w = static_cast<Wrapper*>(this);
    auto&    client = w->client();

    for (index i = 0; i < client.controlChannelsOut().size; i++)
      SETFLOAT(mControlAtoms.data() + i, mControlOutputs[asUnsigned(i)]);

    assert(client.controlChannelsOut().size <= std::numeric_limits<int>::max());
    w->controlOut(static_cast<int>(client.controlChannelsOut().size),
                  mControlAtoms.data());
  }

  float sampleRate() { return sys_getsr(); }

  void makeLatencyOutlet(t_object* pdObject){
    if(mSigIns.size() || mSigOuts.size())
       mLatencyOut = outlet_new(pdObject, gensym("float"));
  }

private:
  static void            doControlOut(Wrapper* x) { x->controlData(); }
  std::vector<ViewType>  mInputs;
  std::vector<ViewType>  mOutputs;
  std::vector<t_sample*> mSigIns;
  std::vector<t_sample*> mSigOuts;
  std::vector<t_float>   mControlOutputs;
  std::vector<t_atom>    mControlAtoms;
  t_outlet*              mLatencyOut;
  t_clock*               mControlClock{nullptr};
  FluidContext           mContext;
};

////////////////////////////////////////////////////////////////////////////////

template <class Wrapper>
struct NonRealTime
{
  NonRealTime()
  {
    auto wrapper = static_cast<Wrapper*>(this);
    mProgressClock = clock_new((t_object*) wrapper, (t_method) checkProcess);
  }

  ~NonRealTime()
  {
    if (mProgressClock) clock_free(mProgressClock);
  }

  static void setup(t_class* c)
  {
    class_addmethod(c, (t_method) callProcess, gensym("bang"), A_NULL);
    
    if(Wrapper::NumInputBuffers)
        class_addmethod(c, (t_method) callBuffer, gensym("buffer"), A_GIMME, 0);
    
    class_addmethod(c, (t_method) callSR, gensym("sr"), A_FLOAT, 0);

    // concurrency messages
    class_addmethod(c, (t_method) callCancel, gensym("cancel"), A_NULL);
    class_addmethod(c, (t_method) doSynchronous, gensym("blocking"), A_FLOAT,
                    0);
    class_addmethod(c, (t_method) doQueueing, gensym("queue"), A_FLOAT, 0);
  }


  void cancel()
  {
    auto& wrapper = static_cast<Wrapper&>(*this);
    auto& client = wrapper.mClient;
    client.cancel();
  }

  void process()
  {
    auto& wrapper = static_cast<Wrapper&>(*this);
    auto& client = wrapper.mClient;
    bool  synchronous = mSynchronous;

    client.setSynchronous(synchronous);
    client.setQueueEnabled(mQueueEnabled);
    client.enqueue(wrapper.mParams);

    Result res = client.process();
    if (wrapper.checkResult(res))
    {
      if (synchronous)
        wrapper.doneBang();
      else
        clockWait();
    }
  }
  static void callCancel(Wrapper* x) { x->cancel(); }
  static void callProcess(Wrapper* x) { x->process(); }

  static void callBuffer(Wrapper* x,t_symbol*,int ac, t_atom* av)
  {
    //deliberate fall through
    switch(ac)
    {
      case 5: pd_typedmess((t_pd*)x,gensym("numchans"), 1 , av + 4);
      case 4: pd_typedmess((t_pd*)x,gensym("startchan"), 1 , av + 3);
      case 3: pd_typedmess((t_pd*)x,gensym("numframes"), 1 , av + 2);
      case 2: pd_typedmess((t_pd*)x,gensym("startframe"), 1 , av + 1);
      case 1: x->doBufferInlet(0, 1, av);
    }
    
     x->process();
  }


  static void checkProcess(Wrapper* x)
  {
    Result res;
    auto&  client = x->mClient;

    ProcessState state = client.checkProgress(res);

    if (state == ProcessState::kDone ||
        state == ProcessState::kDoneStillProcessing)
    {
      if (x->checkResult(res)) x->doneBang();
    }

    if (state != ProcessState::kDone)
    {
      x->progress(client.progress());
      x->clockWait();
    }
  }

  void clockWait()
  {
    clock_delay(mProgressClock, 20); // FIX - set at 20ms for now...
  }

  void setSampleRate(float sr)
  {
    auto& wrapper = static_cast<Wrapper&>(*this);
    mSamplerate = sr > 0 ? sr : mSamplerate;
    wrapper.mParams
        .template forEachParamType<BufferT, Wrapper::template SetBufferSR>(
            &wrapper);
    wrapper.mParams
        .template forEachParamType<InputBufferT, Wrapper::template SetBufferSR>(
            &wrapper);
  }

  float sampleRate() { return mSamplerate <= 0 ? sys_getsr() : mSamplerate; }


  void setupAudio(t_object*, index, index) {}
  void makeLatencyOutlet(t_object*){}

  static void callSR(Wrapper* x, t_float sr) { x->setSampleRate(sr); }

  static void doSynchronous(Wrapper* x, t_float s)
  {
    x->mSynchronous = static_cast<bool>(s);
  }

  static void doQueueing(Wrapper* x, t_float s)
  {
    x->mQueueEnabled = static_cast<bool>(s);
  }

private:
  Wrapper* wrapper() { return static_cast<Wrapper*>(this); }

  t_clock* mProgressClock;
  bool     mSynchronous{true};
  bool     mQueueEnabled{false};
  float    mSamplerate{-1};
};

////////////////////////////////////////////////////////////////////////////////

template <class Wrapper>
struct NonRealTimeAndRealTime : public RealTime<Wrapper>,
                                public NonRealTime<Wrapper>
{
  static void setup(t_class* c)
  {
    RealTime<Wrapper>::setup(c);
    NonRealTime<Wrapper>::setup(c);
  }

  void setupAudio(t_object* pdObject, index numSigIns, index numSigOuts)
  {
    RealTime<Wrapper>::setupAudio(pdObject, numSigIns, numSigOuts);
    NonRealTime<Wrapper>::setupAudio(pdObject, numSigIns, numSigOuts);
  }
};

////////////////////////////////////////////////////////////////////////////////

// Max base type

struct PDBase
{
  t_object* getPDObject() { return &mObject; }

  t_object mObject;
};

////////////////////////////////////////////////////////////////////////////////

// Templates and specialisations for all possible base options

template <class Wrapper, typename NRT, typename RT>
struct FluidPDBase : public PDBase
{
  static_assert(isRealTime<FluidPDBase>::value ||
                    isNonRealTime<FluidPDBase>::value,
                "This object seems to be neither real-time nor non-real-time! "
                "Check that your Client inherits from "
                "Audio[In/Out], Control[In/Out] or Offline[In/Out]");
};

template <class Wrapper>
struct FluidPDBase<Wrapper, std::true_type, std::false_type>
    : public PDBase, public NonRealTime<Wrapper>
{};

template <class Wrapper>
struct FluidPDBase<Wrapper, std::false_type, std::true_type>
    : public PDBase, public RealTime<Wrapper>
{};

template <class Wrapper>
struct FluidPDBase<Wrapper, std::true_type, std::true_type>
    : public PDBase, public NonRealTimeAndRealTime<Wrapper>
{};

////////////////////////////////////////////////////////////////////////////////

} // namespace impl

template <typename Client>
class FluidPDWrapper : public impl::FluidPDBase<FluidPDWrapper<Client>,
                                                typename Client::isNonRealTime,
                                                typename Client::isRealTime>
{
  using WrapperBase =
      impl::FluidPDBase<FluidPDWrapper<Client>, typename Client::isNonRealTime,
                        typename Client::isRealTime>;

  friend impl::RealTime<FluidPDWrapper<Client>>;
  friend impl::NonRealTime<FluidPDWrapper<Client>>;


  template <typename T>
  struct IsThreadedShared : std::false_type
  {};

  template <typename T>
  struct IsThreadedShared<NRTThreadingAdaptor<NRTSharedInstanceAdaptor<T>>>
      : std::true_type
  {};

  struct StreamingListInput
  {

    void processInput(FluidPDWrapper*, long, t_atom*)
    {
    
    }

    void operator()(FluidPDWrapper* x, long ac, t_atom* av)
    {
      FluidContext c;
      
      //todo handle multiple list inlets?
      index count = std::min<index>(x->mListSize, ac);
      for(index i = 0; i < count; ++i)
        x->mInputListData[0][i] = atom_getfloat(av + i);
                           
      x->mClient.process(x->mInputListViews, x->mOutputListViews, c);
      
      for (index i = 0; i <  asSigned(x->mDataOutlets.size()); ++i)
      {
        index count = std::min<index>(x->mListSize,ac); 
        for(index j = 0; j < count; ++j)
        {
          SETFLOAT(x->mOutputListAtoms.data() + j,static_cast<float>(x->mOutputListData[i][j]));
        }
        outlet_list(x->mDataOutlets[asUnsigned(i)],
                    gensym("list"), static_cast<int>(x->mListSize), x->mOutputListAtoms.data());
      }
    }
  };

  struct NoStreamingListInput
  {
    void operator()(FluidPDWrapper*, long, t_atom*) {}
  };

  using ListInputHandler =
      std::conditional_t<isControlIn<typename Client::Client>,
                         StreamingListInput, NoStreamingListInput>;

  ListInputHandler mListHandler;



  template <typename T>
  struct IsModel
  {
    using type = std::false_type;
  };

  template <typename T>
  struct IsModel<NRTThreadingAdaptor<ClientWrapper<T>>>
  {
    using type = typename ClientWrapper<T>::isModelObject;
  };

  template <typename T>
  struct IsModel<ClientWrapper<T>>
  {
    using type = typename ClientWrapper<T>::isModelObject;
  };

  template <typename T>
  using IsModel_t = typename IsModel<T>::type;


  template <size_t N>
  static constexpr auto paramDescriptor()
  {
    return Client::getParameterDescriptors().template get<N>();
  }

  static void printResult(FluidPDWrapper<Client>* x, Result& r)
  {
    if (!x) return;

    if (!r.ok())
    {
      switch (r.status())
      {
      case Result::Status::kWarning:
        if (x->verbose()) object_warn(x, r.message().c_str());
        break;
      case Result::Status::kError:
        pd_error(x, "%s", r.message().c_str());
        break;
      case Result::Status::kCancelled: object_warn(x, "Job cancelled"); break;
      default: {
      }
      }
    }
  }

  bool checkResult(Result& res)
  {
    //    auto& wrapper = static_cast<Wrapper&>(*this);

    if (!res.ok())
    {
      printResult(this, res);
      return false;
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////

  template <size_t N, typename T, typename M, M Method>
  struct FetchValue
  {
    typename T::type operator()(const long ac, t_atom* av, long& currentCount)
    {
      auto defaultValue = paramDescriptor<N>().defaultValue;
      return currentCount < ac ? Method(av + currentCount++) : defaultValue;
    }
  };

  template <size_t N, typename T>
  struct Fetcher;

  template <size_t N>
  struct Fetcher<N, FloatT>
      : public FetchValue<N, FloatT, std::decay_t<decltype(atom_getfloat)>,
                          atom_getfloat>
  {};

  template <size_t N>
  struct Fetcher<N, LongT>
      : public FetchValue<N, LongT, std::decay_t<decltype(atom_getint)>,
                          atom_getint>
  {};


  template <size_t N>
  struct Fetcher<N, StringT>
  {
    std::string operator()(const long ac, t_atom* av, long& currentCount)
    {
      auto defaultValue = paramDescriptor<N>().defaultValue;
      return {currentCount < ac ? atom_getsymbol(av + currentCount++)->s_name
                                : defaultValue};
    }
  };


  /// PD atom to / from Fluid params
  struct ParamAtomConverter
  {

    static t_atomtype atom_gettype(t_atom* a) { return a->a_type; }

    static void atom_setsym(t_atom* a, t_symbol* s)
    {
      a->a_type = A_SYMBOL;
      a->a_w.w_symbol = s;
    }

    static void atom_setfloat(t_atom* atom, float v)
    {
      (atom)->a_type = A_FLOAT;
      (atom)->a_w.w_float = v;
    }

    static std::string getString(t_atom* a)
    {
      switch (atom_gettype(a))
      {
      case A_FLOAT: return std::to_string(atom_getfloat(a));
      default: {
        char result[MAXPDSTRING];
        atom_string(a, &result[0], MAXPDSTRING);
        return result;
      }
      }
    }

    template <typename T>
    static std::enable_if_t<std::is_integral<T>::value, T>
    fromAtom(FluidPDWrapper* /*x*/, t_atom* a, T)
    {
      return atom_getint(a);
    }

    template <typename T>
    static std::enable_if_t<std::is_floating_point<T>::value, T>
    fromAtom(FluidPDWrapper* /*x*/, t_atom* a, T)
    {
      return atom_getfloat(a);
    }

    static auto fromAtom(FluidPDWrapper* x, t_atom* a, const BufferT::type& )
    {
      return BufferT::type(
          new PDBufferAdaptor(atom_getsymbol(a), x->sampleRate()));
    }

    static auto fromAtom(FluidPDWrapper* x, t_atom* a, const InputBufferT::type&)
    {
      return InputBufferT::type(
          new PDBufferAdaptor(atom_getsymbol(a), x->sampleRate()));
    }

    static auto fromAtom(FluidPDWrapper*, t_atom* a, const StringT::type&)
    {
      auto s = getString(a);
      return s;
    }

    template <typename T>
    static std::enable_if_t<IsSharedClient<T>::value, T>
    fromAtom(FluidPDWrapper*, t_atom* a, T)
    {
      return {atom_getsymbol(a)->s_name};
    }

    template<typename T> 
    static Optional<T> fromAtom(FluidPDWrapper* x, t_atom* a, const Optional<T>) 
    {
      return {fromAtom(x,a,T{})}; 
    }
    
    template <typename T>
    static std::enable_if_t<std::is_integral<T>::value> toAtom(t_atom* a, T v)
    {
      atom_setfloat(a, v);
    }

    template <typename T>
    static std::enable_if_t<std::is_floating_point<T>::value> toAtom(t_atom* a,
                                                                     T       v)
    {
      atom_setfloat(a, static_cast<float>(v));
    }

    static auto toAtom(t_atom* a, BufferT::type v)
    {
      auto b = static_cast<PDBufferAdaptor*>(v.get());
      atom_setsym(a, b ? b->name() : nullptr);
    }

    static auto toAtom(t_atom* a, InputBufferT::type v)
    {
      auto b = static_cast<const PDBufferAdaptor*>(v.get());
      atom_setsym(a, b ? b->name() : nullptr);
    }

    static auto toAtom(t_atom* a, StringT::type v)
    {
      atom_setsym(a, gensym(v.c_str()));
    }

    static auto toAtom(t_atom* a, FluidTensor<std::string, 1> v)
    {
      for (auto& s : v) atom_setsym(a++, gensym(s.c_str()));
    }

    template <typename T>
    static std::enable_if_t<std::is_floating_point<T>::value>
    toAtom(t_atom* a, FluidTensor<T, 1> v)
    {
      for (auto& x : v) { atom_setfloat(a++, static_cast<float>(x)); }
    }

    template <typename T>
    static std::enable_if_t<std::is_integral<T>::value>
    toAtom(t_atom* a, FluidTensor<T, 1> v)
    {
      for (auto& x : v) atom_setfloat(a++, x);
    }

    template <typename T>
    static std::enable_if_t<IsSharedClient<T>::value> toAtom(t_atom* a, T v)
    {
      atom_setsym(a, gensym(v.name()));
    }

    template <typename... Ts, size_t... Is>
    static void toAtom(t_atom* a, std::tuple<Ts...>&& x,
                       std::index_sequence<Is...>,
                       std::array<size_t, sizeof...(Ts)> offsets)
    {
      (void) std::initializer_list<int>{
          (toAtom(a + offsets[Is], std::get<Is>(x)), 0)...};
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Setter

  template <typename T, size_t N>
  struct Setter
  {
    static constexpr index argSize = paramDescriptor<N>().fixedSize;

    static void set(FluidPDWrapper<Client>* x, t_symbol*, int ac, t_atom* av)
    {
//      NOTUSED(s);

      ParamLiteralConvertor<T, argSize> a;
      a.set(Client::getParameterDescriptors().template makeValue<N>());

      x->messages().reset();

      for (index i = 0; i < argSize && i < ac; i++)
        a[i] = ParamAtomConverter::fromAtom(x, av + i, a[0]);

      x->params().template set<N>(a.value(),
                                  x->verbose() ? &x->messages() : nullptr);
      printResult(x, x->messages());
    }
  };

  template <size_t N>
  struct Setter<LongArrayT, N>
  {

    static void set(FluidPDWrapper<Client>* x, t_symbol*, int ac, t_atom* av)
    {
      x->messages().reset();
      typename LongArrayT::type& a = x->params().template get<N>();

      a.resize(ac);

      using T = typename LongArrayT::type::type;

      for (index i = 0; i < static_cast<index>(ac); i++)
        a[i] = ParamAtomConverter::fromAtom(x, av + i, T{});
    }
  };
  

  template <size_t N>
  struct Setter<LongRuntimeMaxT,N>
  {
    static void set(FluidPDWrapper<Client>* x, t_symbol*, int ac, t_atom* av)
    {
      if (!ac) return;
      x->messages().reset();
      auto& a = x->params().template get<N>();

      if (!x->mInitialized)        
        x->params().template set<N>(
            LongRuntimeMaxParam(atom_getint(av), a.maxRaw()),
            x->verbose() ? &x->messages() : nullptr);
      else
        x->params().template set<N>(
            LongRuntimeMaxParam(atom_getint(av), a.max()),
            x->verbose() ? &x->messages() : nullptr);

      printResult(x, x->messages());
    }
  };
  
  template <size_t N>
  struct Setter<FFTParamsT,N>
  {
    static void set(FluidPDWrapper<Client>* x, t_symbol*, int ac, t_atom* av)
    {
      if (!ac) return;
      x->messages().reset();
      auto& a = x->params().template get<N>();
      
      std::array<index,3> defaults{1024, -1, -1};
      
      for (index i = 0; i < 3 && i < static_cast<index>(ac); i++)
            defaults[asUnsigned(i)] = atom_getint(av + i);
      
      if(!x->mInitialized)
        a = FFTParams(defaults[0], defaults[1], defaults[2], a.maxRaw());
      else
        x->params().template set<N>(FFTParams(defaults[0], defaults[1], defaults[2], a.max()), x->verbose() ? &x->messages() : nullptr);
      
      printResult(x, x->messages());
    }
  };
  

  template <size_t N>
  struct Setter<ChoicesT, N>
  {

    static void set(FluidPDWrapper<Client>* x,t_symbol*, int ac, t_atom* av)
    {
      x->messages().reset();
      auto desc = x->params().template descriptorAt<N>();
            
      typename ChoicesT::type a{ac ? 0u : desc.defaultValue};
      

      for (index i = 0; i < static_cast<index>(ac); i++)
      {
          std::string s = ParamAtomConverter::fromAtom(x,av + i, std::string{});
          index pos = desc.lookup(s);
          if(pos == -1)
          {
            pd_error(x->getPDObject(),"%s: unrecognised choice",s.c_str());
            continue;
          }
          a.set(asUnsigned(pos),1);
      }
      
      x->params().template set<N>(std::move(a), x->verbose() ? &x->messages() : nullptr);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Getter

  template <typename T, size_t N>
  struct Getter
  {
    static constexpr size_t argSize = paramDescriptor<N>().fixedSize;

    static std::vector<t_atom> get(FluidPDWrapper<Client>* x)
    {
      ParamLiteralConvertor<T, argSize> a;
      std::vector<t_atom>               result(argSize);

      a.set(x->params().template get<N>());

      for (auto i = 0u; i < argSize; i++)
        ParamAtomConverter::toAtom(result.data() + i, a[i]);

      return result;
    }
  };


public:
  using ClientType = Client;
  using ParamDescType = typename Client::ParamDescType;
  using ParamSetType = typename Client::ParamSetType;
  using ParamValues = typename ParamSetType::ValueTuple;
  static constexpr index NumInputBuffers = ParamDescType::template NumOfType<InputBufferT>;
  static constexpr index NumOutputBuffers = ParamDescType::template NumOfType<BufferT>;
  
  static t_class* getInletProxyClass(t_class* setClass = nullptr)
  {
    static t_class* c;
    return (c = setClass ? setClass : c);
  }
  
  struct InletProxy
  {
    t_pd obj;
    FluidPDWrapper* mOwner;
    index mIndex;
  };

  static void inletProxyInit(InletProxy* x, FluidPDWrapper* owner, index idx)
  {
    x->obj = getInletProxyClass();
    x->mOwner = owner;
    x->mIndex = idx;
  }
  
  static void inletProxyBuffer(InletProxy* x, t_symbol*, int ac, t_atom* av)
  {
    x->mOwner->doBufferInlet(x->mIndex + 1, ac, av);
  }
  
  static void inletProxySetup(const char* className)
  {
    std::string inletProxyClassName = std::string(className) + " inlet proxy";
    getInletProxyClass(class_new(gensym(inletProxyClassName.c_str()),0, 0, sizeof(InletProxy), 0, A_NULL));
    class_addmethod(getInletProxyClass(), (t_method) inletProxyBuffer, gensym("buffer"), A_GIMME, 0);
  }
  
  FluidPDWrapper(t_symbol*, int ac, t_atom* av)
      : mListSize{32},
        mNRTProgressOutlet{nullptr},
        mControlOutlet(nullptr),
        mParams(Client::getParameterDescriptors()),
        mParamSnapshot(mParams.toTuple()),
        mClient{initParamsFromArgs(ac, av)},mCanvas{canvas_getcurrent()}
  {
    t_object* pdObject = impl::PDBase::getPDObject();

    auto results = mParams.keepConstrained(true);
    mParamSnapshot = mParams.toTuple();

    for (auto& r : results) printResult(this, r);

    this->setupAudio(pdObject, mClient.audioChannelsIn(),
                     mClient.audioChannelsOut());

    if (index new_ins = mClient.controlChannelsIn())
    {
      mAutosize = true;
      if (mListSize)
      {
        mInputListData.resize(new_ins, mListSize);
        for (index i = 1; i <= new_ins; ++i)
          mInputListViews.emplace_back(mInputListData.row(i - 1));
      }

      mProxies.reserve(asUnsigned(new_ins));
      for (index i = 1; i < new_ins; ++i)
      {
        mProxies.push_back((InletProxy*)pd_new(getInletProxyClass()));
        InletProxy* proxy = mProxies.back();
        inletProxyInit(proxy,this, i);
        inlet_new(pdObject, &pdObject->ob_pd, 0, 0);
      }
    }
    
    if(mProxies.size() < NumInputBuffers)
    {
      for(index i = asSigned(mProxies.size()); i < NumInputBuffers - 1; i++)
      {
        mProxies.push_back((InletProxy*)pd_new(getInletProxyClass()));
        InletProxy* proxy = mProxies.back();
        inletProxyInit(proxy,this, i);
        inlet_new(pdObject,&proxy->obj,0,0);
      }
    }
    
    
    //how many non-signal outlets do we need?
    index numDataOutlets = std::max<index>({NumOutputBuffers,mClient.controlChannelsOut().count,
      Client::getMessageDescriptors().size() > 0
    });
    
    for(index i = 0; i < numDataOutlets; ++i)
      mDataOutlets.push_back(outlet_new(pdObject,nullptr));
    
    if (isNonRealTime<Client>::value ||
        (IsModel_t<Client>::value && Client::isRealTime::value))
    {
      mNRTProgressOutlet = outlet_new(pdObject, gensym("float"));
    }

    if (mClient.controlChannelsOut().count) 
    {
      if(mListSize)
      {
        mOutputListData.resize(mClient.controlChannelsOut().count, mListSize);
        mOutputListAtoms.reserve(asUnsigned(mListSize));
        for (index i = 0; i < mClient.controlChannelsOut().count; ++i)
          mOutputListViews.emplace_back(mOutputListData.row(i));
      }
      mControlOutlet = mDataOutlets[0];
    }              

    const auto& m = Client::getMessageDescriptors();
    
    this->makeLatencyOutlet(pdObject);
    
    mParams.template forEachParamType<InputBufferT>([this](auto&, auto idx){
      static constexpr index N = decltype(idx)::value;
      mBufferDispatch.push_back([](FluidPDWrapper* x, int ac, t_atom* av)
      {
          Setter<InputBufferT, N>::set(x, nullptr, ac, av);
      });
    });
    
    
    //setup an array of buffer~ object that we'll use if the respective params are unset when process is called
    
    //dirty: reserve in advance so that it doesn't need to reallocate and mess up pointers to contents. 
    mHostedOutputBufferObjects.reserve(NumOutputBuffers);
    
    mParams.template forEachParamType<BufferT>([this](auto&, auto idx){
        constexpr index N = idx();
        std::string name = uniqueName();
        mHostedOutputBufferObjects.push_back(impl::ArrayManager(gensym(name.c_str())));
          
          
        auto currentValue =
            static_cast<PDBufferAdaptor*>(mParams.template get<N>().get());

        if(!currentValue || currentValue->name() == gensym(""))
          mParams.template set<N>(BufferT::type(new PDBufferAdaptor(gensym(name.c_str()),sys_getsr(),&mHostedOutputBufferObjects.back())),nullptr);
    });
    
    mInitialized = true; 
  }
  
  ~FluidPDWrapper()
  {
    for (auto&& s: mArgObjects){
      t_symbol* name = s.second;
      if(name)
      {
        if(name->s_thing)
        {
          t_pd* o = (t_pd*)name->s_thing;
          pd_unbind(o, name);
          pd_free(o);
        }
      }
    }
  }

  void doneBang()
  {
    
    static t_symbol* buffer_sym = gensym("buffer");
     
    mParams.template forEachParamType<BufferT>([this, n = 0u](auto&, auto idx) mutable
    {
        static constexpr index N = idx();
        auto b = static_cast<PDBufferAdaptor*>(params().template get<N>().get());
        
        t_atom a;
        a.a_type = A_SYMBOL;
        a.a_w.w_symbol = b->name();
        
        outlet_anything(mDataOutlets[n++], buffer_sym, 1, &a);
    });
  }

  void controlOut(int ac, t_atom* av)
  {
    outlet_list(mControlOutlet, nullptr, static_cast<short>(ac), av);
  }

  static bool isTag(t_atom* a)
  {
    t_symbol* s = atom_getsymbol(a);

    return a->a_type == A_SYMBOL && s && strlen(s->s_name) > 1 &&
           s->s_name[0] == '-';
  }

  static int findTag(int start, int ac, t_atom* av)
  {
    for (int i = start; i < ac; i++)
      if (isTag(av + i)) return i;

    return ac;
  }

  static int paramArgOffset(int ac, t_atom* av) { return findTag(0, ac, av); }

  template <size_t N, typename T>
  struct MatchName
  {
    void operator()(const typename T::type&, const char* name,
                    bool& matched)
    {
      std::string paramName = lowerCase(paramDescriptor<N>().name);

      if (!strcmp(paramName.c_str(), name))
      {
        matched = true;
        return;
      }
      
      if constexpr (std::is_same<LongRuntimeMaxT,T>::value)
      {
        std::string maxParamName = "max" + paramName;

        if (!strcmp(maxParamName.c_str(), name))
        {
          matched = true;
          return;
        }
      }
      
      if constexpr (std::is_same<FFTParamsT,T>::value)
      {
        std::string maxParamName = "maxfftsize";

        if (!strcmp(maxParamName.c_str(), name))
        {
          matched = true;
          return;
        }
      }
    }
  };

  void paramArgProcess(int ac, t_atom* av)
  {
    int tag = paramArgOffset(ac, av);

    while (tag < ac)
    {
      t_symbol* s = atom_getsymbol(av + tag);
      int       endTag = findTag(tag + 1, ac, av);

      bool matched = false;
      mParams.template forEachParam<MatchName>(s->s_name + 1, matched);
      if (!strcmp(s->s_name + 1, "warnings")) matched = true;

      if (isNonRealTime<Client>::value && !strcmp(s->s_name + 1, "blocking"))
        matched = true;

      if (isNonRealTime<Client>::value && !strcmp(s->s_name + 1, "queue"))
        matched = true;

      if (isNonRealTime<Client>::value && !strcmp(s->s_name + 1, "sr"))
        matched = true;

      if (!matched)
      {
        pd_error(impl::PDBase::getPDObject(), "No parameter named %s",
                 s->s_name + 1);
      }
      else
      {
        pd_typedmess((t_pd*) impl::PDBase::getPDObject(), gensym(s->s_name + 1),
                     endTag - (tag + 1), av + tag + 1);
      }

      tag = endTag;
    }
  }  

  static void* create(t_symbol* sym, int ac, t_atom* av)
  {
    void* x = pd_new(getClass());
    new (x) FluidPDWrapper(sym, ac, av);
    
    static constexpr bool hasListInput = isControlIn<typename Client::Client>;
    
    if (static_cast<index>(paramArgOffset(ac, av)) >
        ParamDescType::NumFixedParams + ParamDescType::NumPrimaryParams + hasListInput)
    {
      impl::object_warn(x, "Too many arguments. Got %d, expect at most %d", ac,
                        ParamDescType::NumFixedParams + ParamDescType::NumPrimaryParams + hasListInput);
    }

    return x;
  }

  static void destroy(FluidPDWrapper* x) { x->~FluidPDWrapper(); }

  static void makeClass(const char* className)
  {
    const ParamDescType& p = Client::getParameterDescriptors();
    const auto&          m = Client::getMessageDescriptors();

    getClass(class_new(gensym(className), (t_newmethod) create,
                       (t_method) destroy, sizeof(FluidPDWrapper), 0, A_GIMME,
                       0));
    WrapperBase::setup(getClass());
    
    class_addmethod(getClass(), (t_method) doReset, gensym("reset"), A_NULL);
    class_addmethod(getClass(), (t_method) doWarnings, gensym("warnings"),
                    A_FLOAT, 0);
    class_addmethod(getClass(), (t_method) doVersion, gensym("version"),
                    A_NULL);
    makeReferable(); 
    
    if (isControlIn<typename Client::Client>)
    {
      class_addmethod(getClass(), (t_method) handleList, gensym("list"), A_GIMME, 0);
    }


    m.template iterate<SetupMessage>();

    p.template iterateMutable<SetupParameter>();
    class_sethelpsymbol(getClass(), gensym(className));    
    inletProxySetup(className);
  }

  static void doVersion(FluidPDWrapper*)
  {
    post("Fluid Corpus Manipulation version %s", fluidVersion());
  }

  static void doReset(FluidPDWrapper* x)
  {
    x->mParams.fromTuple(x->mParamSnapshot);
  }

  static void doWarnings(FluidPDWrapper* x, t_float warnings)
  {
    x->mVerbose = (bool) warnings;
  }

  template<class T>
  using HasProcess_t = decltype(std::declval<T&>().process());


  
  void doBufferInlet(index inlet, int ac, t_atom* av)
  {
    mBufferDispatch[inlet](this, ac, av);
  }
  

  Result&       messages() { return mResult; }
  bool          verbose() { return mVerbose; }
  Client&       client() { return mClient; }
  ParamSetType& params() { return mParams; }


  void progress(double progress)
  {
    outlet_float(mNRTProgressOutlet, static_cast<float>(progress));
  }


  void printBufferSRs()
  {
    mParams.template forEachParamType<BufferT, PrintBufferSR>(this);
    mParams.template forEachParamType<InputBufferT, PrintBufferSR>(this);
  }

  // Set the sample rate of a PD buffer adaptor param
  template <size_t N, typename T>
  struct SetBufferSR
  {
    void operator()(const typename T::type& param, FluidPDWrapper* x)
    {
      if (auto b = static_cast<const PDBufferAdaptor*>(param.get()))
        b->sampleRate(x->sampleRate());
    }
  };


  void resizeListHandlers(index newSize)
  {
      index numIns = mClient.controlChannelsIn();
      mListSize = newSize; 
      if(mListSize)
      {
        mInputListData.resize(numIns,mListSize);
        mInputListViews.clear();
        for (index i = 0; i < numIns; ++i)
        {
          mInputListViews.emplace_back(mInputListData.row(i));
        }
        
        mOutputListData.resize(mClient.controlChannelsOut().count,mListSize);
        mOutputListAtoms.reserve(asUnsigned(mListSize));
        mOutputListViews.clear();
        for (index i = 0; i < mClient.controlChannelsOut().count; ++i)
        {
          mOutputListViews.emplace_back(mOutputListData.row(i));
        }
        
      }
  }

  static void doList(FluidPDWrapper* x, t_symbol*, long ac, t_atom* av)
  {
    if(ac != x->mListSize) x->resizeListHandlers(ac);
    x->mListHandler(x, ac, av);
  }
  
  static void handleList(FluidPDWrapper* x, t_symbol* s, long ac, t_atom* av)
  {
      doList(x,s,ac,av);      
  }

private:
  static t_class* getClass(t_class* setClass = nullptr)
  {
    static t_class* c = nullptr;
    return (c = setClass ? setClass : c);
  }

  static std::string lowerCase(const char* str)
  {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
  }
  
  static std::string uniqueName()
  {
    std::stringstream ss;
    const auto p1 = std::chrono::steady_clock::now();
    ss << 'u' << std::chrono::duration_cast<std::chrono::nanoseconds>(
                   p1.time_since_epoch()).count();
    return ss.str();
  }

  ParamSetType& initParamsFromArgs(int ac, t_atom* av)
  {
    // Process arguments for instantiation parameters
    if (long numArgs = paramArgOffset(ac, av))
    {
      long argCount{0};
      
      if(isControlIn<typename Client::Client>)
      {
        mListSize = atom_getint(av);
        numArgs -= 1;
        av += 1;
        ac--;
      }

      mParams.setPrimaryParameterValues(
          true,
          [](auto idx, long ac, t_atom* av, long& currentCount) {
            auto defaultValue = paramDescriptor<idx()>().defaultValue;

            if constexpr (std::is_same<std::decay_t<decltype(defaultValue)>,
                                       LongRuntimeMaxParam>())
            {
              index val = currentCount < ac ? atom_getint(av + currentCount++)
                                            : defaultValue();
              return LongRuntimeMaxParam{val, -1};
            }
            else
            {
              return currentCount < ac ? atom_getint(av + currentCount++)
                                       : defaultValue;
            }
          },
          numArgs, av, argCount);
//      for (auto& r : r1) x->mMessages.push_back(r);

      mParams.template setFixedParameterValues<Fetcher>(
          true, numArgs, av, argCount);
      // for (auto& r : results) mMessages.push_back(r);    
    }
    // process in-box attributes for mutable params
    paramArgProcess(ac, av);
    checkName(mParams);
    // return params so this can be called in client initaliser
    return mParams;
  }
  
  template<typename ClientType = Client>
  std::enable_if_t<IsThreadedShared<ClientType>::value>
  checkName(ParamSetType& params)
  {
    if(params.template get<0>().size() == 0)
    {
      params.template set<0>(uniqueName() ,nullptr);
    }
  }

  template<typename ClientType = Client>
  std::enable_if_t<!IsThreadedShared<ClientType>::value>
  checkName(ParamSetType&){}

  template <typename CType = Client>
  static std::enable_if_t<!IsThreadedShared<CType>::value> makeReferable()
  {}

  template <typename CType = Client>
  static std::enable_if_t<IsThreadedShared<CType>::value> makeReferable()
  {
    class_addmethod(getClass(), (t_method) doSharedClientRefer, gensym("refer"), A_DEFSYM,
                    0);
  }

  static void doSharedClientRefer(FluidPDWrapper* x, t_symbol* newName)
  {
    std::string name(newName->s_name);
    if (std::string(name) != x->mParams.template get<0>())
    {
      Result r = x->mParams.lookup(name);
      if (r.ok())
      {
        x->mParams.refer(name);
        x->mClient = Client(x->mParams);
      }
      else
        printResult(x, r);
    }
  }


  // Sets up a single parameter

  template <size_t N, typename T>
  struct SetupParameter
  {
    void operator()(const T& parameter)
    {
      std::string name = lowerCase(parameter.name);
      t_method    setterMethod = (t_method) &Setter<T, N>::set;

      class_addmethod(getClass(), setterMethod, gensym(name.c_str()), A_GIMME,
                      0);

      using SetterFn = void (*)(FluidPDWrapper<Client>* x, t_symbol*, int ac, t_atom* av);
      
      if constexpr (std::is_same<T, LongRuntimeMaxT>::value)
      {
        std::string maxname = "max" + name;
                
        SetterFn maxSetter = [](FluidPDWrapper<Client>* x, t_symbol*, int ac, t_atom* av)
        {
          static constexpr index Idx = N;
          if(ac && !x->mInitialized)
          {
            auto current = x->mParams.template get<Idx>();
            index newMax = atom_getint(av);
            if(newMax > 0)
            {
              x->mParams.template set<Idx>(LongRuntimeMaxParam(current(),newMax),nullptr);
            }
          }
        };
        
        class_addmethod(getClass(),(t_method)maxSetter,gensym(maxname.c_str()), A_GIMME, 0);
      }
      
      if constexpr (std::is_same<T,FFTParamsT>::value)
      {
        std::string maxname = "maxfftsize";
        
        SetterFn maxSetter = [](FluidPDWrapper<Client>* x, t_symbol*, int ac, t_atom* av)
        {
          static constexpr index Idx = N;
          if(ac && !x->mInitialized)
          {
            auto current = x->mParams.template get<Idx>();
            index newMax = atom_getint(av);
            if(newMax > 0)
            {
              x->mParams.template set<Idx>(FFTParams(current.winSize(), current.hopRaw(), current.fftRaw(),newMax),nullptr);
            }
          }
        };
        
        class_addmethod(getClass(),(t_method)maxSetter,gensym(maxname.c_str()), A_GIMME, 0);
      }
    }
  };

  template <size_t N, typename T>
  struct PrintBufferSR
  {
    void operator()(const typename T::type& param, FluidPDWrapper*)
    {
      if (auto b = static_cast<const PDBufferAdaptor*>(param.get()))
        post("%f", b->sampleRate());
    }
  };

  // Calculate output size of messages
  template <typename T>
  static size_t ResultSize(T)
  {
    return 1;
  }

  template <template <typename, size_t> class Tensor, typename T>
  static size_t ResultSize(Tensor<T, 1>&& x)
  {
    return asUnsigned(static_cast<FluidTensor<T, 1>>(x).size());
  }

  template <typename... Ts, size_t... Is>
  static std::tuple<std::array<size_t, sizeof...(Ts)>, size_t>
  ResultSize(std::tuple<Ts...>&& x, std::index_sequence<Is...>)
  {
    size_t                            size = 0;
    std::array<size_t, sizeof...(Ts)> offsets;
    (void) std::initializer_list<int>{
        (offsets[Is] = size, size += ResultSize(std::get<Is>(x)), 0)...};
    return std::make_tuple(offsets, size);
  }

  template <typename T>
  static std::enable_if_t<!isSpecialization<T, std::tuple>::value>
  messageOutput(FluidPDWrapper* x, t_symbol* s,std::vector<t_atom>& outputTokens, MessageResult<T> r)
  {
    size_t  resultSize = ResultSize(static_cast<T>(r));
    
    resultSize += outputTokens.size();
    std::vector<t_atom> out(resultSize);
    std::copy_n(outputTokens.begin(), outputTokens.size(), out.begin());
    ParamAtomConverter::toAtom(out.data() + outputTokens.size(), static_cast<T>(r));

    outlet_anything(x->mDataOutlets[0], s, static_cast<int>(resultSize),
                    out.data());
  }

  template <typename... Ts>
  static void messageOutput(FluidPDWrapper* x, t_symbol* s,std::vector<t_atom>& outputTokens,
                            MessageResult<std::tuple<Ts...>> r)
  {
    auto   indices = std::index_sequence_for<Ts...>();
    size_t resultSize;
    std::array<size_t, sizeof...(Ts)> offsets;
    std::tie(offsets, resultSize) =
        ResultSize(static_cast<std::tuple<Ts...>>(r), indices);
    
    resultSize += outputTokens.size();
    std::vector<t_atom> out(resultSize);
    std::copy_n(outputTokens.begin(), outputTokens.size(), out.begin());
    ParamAtomConverter::toAtom(out.data() + outputTokens.size(), static_cast<std::tuple<Ts...>>(r),
                               indices, offsets);
                               
    outlet_anything(x->mDataOutlets[0], s, static_cast<int>(resultSize),
                    out.data());
  }

  static void messageOutput(FluidPDWrapper* x, t_symbol* s, std::vector<t_atom>& outputTokens, MessageResult<void>)
  {
    outlet_anything(x->mDataOutlets[0], s, static_cast<int>(outputTokens.size()), outputTokens.data());
  }

  //////////////////////////////
  /// message registration


  template <size_t N, typename T>
  struct SetupMessage
  {
    void operator()(const T& message)
    {
      if (message.name == "load")
      {
        SpecialCase<MessageResult<void>, std::string>{}.template handle<N>(
            typename T::ReturnType{}, typename T::ArgumentTypes{},
            [](auto) {
              //              class_addmethod(getClass(),
              //                              (method)
              //                              deferLoad<decltype(M)::value>,
              //                              lowerCase(message.name).c_str(),
              //                              A_GIMME, 0);
            });
        return;
      }
      if (message.name == "dump")
      {
        SpecialCase<MessageResult<std::string>>{}.template handle<N>(
            typename T::ReturnType{}, typename T::ArgumentTypes{},
            [](auto) {
              //              class_addmethod(getClass(),
              //                              (method)
              //                              deferDump<decltype(M)::value>,
              //                              lowerCase(message.name).c_str(),
              //                              A_GIMME, 0);
            });
        return;
      }
      if (message.name == "print")
      {
        SpecialCase<MessageResult<std::string>>{}.template handle<N>(
            typename T::ReturnType{}, typename T::ArgumentTypes{},
            [&message](auto M) {
              class_addmethod(
                  getClass(), (t_method) doPrint<decltype(M)::value>,
                  gensym(lowerCase(message.name).c_str()), A_GIMME, 0);
            });
        return;
      }
      if (message.name == "read")
      {
        using ReturnType = typename T::ReturnType;
        constexpr bool isVoid = std::is_same<ReturnType, MessageResult<void>>::value;
        
        using IfVoid = SpecialCase<MessageResult<void>,std::string>;
        using IfParams = SpecialCase<MessageResult<ParamValues>,std::string>;
        using Handler = std::conditional_t<isVoid, IfVoid, IfParams>;
        
        Handler{}.template handle<N>(
            typename T::ReturnType{}, typename T::ArgumentTypes{},
            [&message](auto M) {
              class_addmethod(getClass(), (t_method) doRead<decltype(M)::value>,
                              gensym(lowerCase(message.name).c_str()), A_GIMME,
                              0);
            });
        return;
      }
      if (message.name == "write")
      {
        SpecialCase<MessageResult<void>, std::string>{}.template handle<N>(
            typename T::ReturnType{}, typename T::ArgumentTypes{},
            [&message](auto M) {
              class_addmethod(
                  getClass(), (t_method) doWrite<decltype(M)::value>,
                  gensym(lowerCase(message.name).c_str()), A_GIMME, 0);
            });
        return;
      }
      class_addmethod(getClass(), (t_method) invokeMessage<N>,
                      gensym(lowerCase(message.name).c_str()), A_GIMME, 0);
    }

    // This amounts to me really, really promising the compiler that it's all ok
    //(life isn't as simple as being able to runtime dispatch on message names,
    // I neeed to make sure messages whose sigs don't match the special case
    // don't get even the possibility of being run)
    template <typename Return, typename... Args>
    struct SpecialCase
    {
      template <size_t M, typename F>
      void handle(Return, std::tuple<Args...>, F&& f)
      {
        f(std::integral_constant<size_t, M>());
      }

      template <size_t M, typename U, typename ArgTuple, typename F>
      void handle(U, ArgTuple, F&&)
      {}
    };
  };

  template <size_t N>
  static void doPrint(FluidPDWrapper* x, t_symbol*, long, t_atom*)
  {
    auto result = x->mClient.template invoke<N>(x->mClient);
    if (x->checkResult(result))
    {
      poststring(/*(t_object*) x, "%s",*/
                 static_cast<std::string>(result).c_str());
      outlet_anything(x->mDataOutlets.back(), gensym("print"), 0, nullptr);
    }
  }

  template <size_t N>
  static void doRead(FluidPDWrapper* x, t_symbol*, long ac, t_atom* av)
  {
    if (!ac) return;
    const char* filename = av[0].a_w.w_symbol->s_name;
    if (!filename)
    {
      pd_error(x, "Missing or invalid filename");
      return;
    }
    char buf[MAXPDSTRING], *bufptr;
    int  fd =
        canvas_open(x->mCanvas, filename, "", buf, &bufptr, MAXPDSTRING, 0);
    if (fd < 0)
    {
      pd_error(x, "File not found");
      return;
    }
    sys_close(fd);
    std::string complete(buf);
    complete += "/";
    complete += bufptr;
    auto messageResult = x->mClient.template invoke<N>(x->mClient, complete);

    if (x->checkResult(messageResult))
    {
      outlet_anything(x->mDataOutlets.back(), gensym("read"), 0, nullptr);
    }
  }

  template <size_t N>
  static void doWrite(FluidPDWrapper* x, t_symbol*, long ac, t_atom* av)
  {
    if (!ac) return;
    const char* filename = av[0].a_w.w_symbol->s_name;
    if (!filename)
    {
      pd_error(x, "Missing or invalid filename");
      return;
    }
    char filenamebuf[MAXPDSTRING];
    char buf2[MAXPDSTRING];

    // BIG YIKES (this is what PD does in d_soundfile)
    strncpy(filenamebuf, filename, MAXPDSTRING);
    filenamebuf[MAXPDSTRING - 10] = 0;
    canvas_makefilename(x->mCanvas, filenamebuf, buf2, MAXPDSTRING);

    auto messageResult = x->mClient.template invoke<N>(x->mClient, buf2);

    if (x->checkResult(messageResult))
    {
      outlet_anything(x->mDataOutlets[0], gensym("write"), 0, nullptr);
    }
  }

  ///////////////////////////////
  /// message invocation
  template <size_t N>
  static void invokeMessage(FluidPDWrapper* x, t_symbol* s, long ac, t_atom* av)
  {
    using IndexList =
        typename Client::MessageSetType::template MessageDescriptorAt<
            N>::IndexList;
    x->client().setParams(x->params());
    invokeMessageImpl<N>(x, s, ac, av, IndexList());
  }

  template <size_t N, size_t... Is>
  static void invokeMessageImpl(FluidPDWrapper* x, t_symbol* s, long ac,
                                t_atom* av, std::index_sequence<Is...>)
  {
    using ArgTuple =
        typename Client::MessageSetType::template MessageDescriptorAt<
            N>::ArgumentTypes;

    // Read in arguments
    ArgTuple args{setArg<ArgTuple, N, Is>(x, ac, av)...};
    std::vector<t_atom> outputTokens;

    ForEach(args,[&outputTokens](auto& a){
        outputToken(outputTokens,a);
    });


    auto result =
        x->mClient.template invoke<N>(x->mClient, std::get<Is>(args)...);

    if (x->checkResult(result)) messageOutput(x, s, outputTokens, result);
  }

  template <typename Tuple, size_t N, size_t I>
  static auto setArg(FluidPDWrapper* x, long ac, t_atom* av)
  {
    if (I < asUnsigned(ac))
      return ParamAtomConverter::fromAtom(
          x, av + I, typename std::tuple_element<I, Tuple>::type{});
    else
      return x->argDefault<N, I>(typename std::tuple_element<I, Tuple>::type{});
  }
  
  template<size_t N, size_t I, typename T>
  auto argDefault(T obj){
     return obj;
  }
  
  template<size_t N, size_t I>
  std::shared_ptr<BufferAdaptor> argDefault(std::shared_ptr<BufferAdaptor>&&){

    t_symbol*& argName = mArgObjects[{N,I}];
    
    if(!argName)
    {
      argName = gensym(uniqueName().c_str());
      t_atom oNameAtom;
      SETSYMBOL(&oNameAtom, argName);
      
      mArgBuffers.emplace(std::make_pair(std::make_pair(N,I), impl::ArrayManager(argName)));
    }
     
    
    auto bufferManager  = mArgBuffers.find({N,I});
    assert(bufferManager != mArgBuffers.end());
    if(bufferManager != mArgBuffers.end())
      return std::make_shared<PDBufferAdaptor>(argName,sys_getsr(), &(bufferManager->second));
    else
      return std::make_shared<PDBufferAdaptor>(argName,sys_getsr());
  }
  
  template<size_t N, size_t I, typename T>
  SharedClientRef<T> argDefault(SharedClientRef<T>)
  {
    t_symbol*& argName = mArgObjects[{N,I}];

    if(!argName)
    {
      argName = gensym(uniqueName().c_str());
      t_atom oNameAtom;
      SETSYMBOL(&oNameAtom, argName);
      t_symbol* pd_class = gensym(SharedClientName<T>);
          
      using t_newgimme = t_pd* (*) (t_symbol * s, int argc, t_atom* argv);
      static t_newgimme fun = (t_newgimme) zgetfn(&pd_objectmaker, pd_class);
      
      if(fun)
      {
        t_atom name;
        SETSYMBOL(&name,argName);
        t_pd* newobj = fun(pd_class, 1, &name);
        pd_bind(newobj, argName);
      }
    }

    return {argName->s_name};
  }
  
  template<size_t N,typename T>
  SharedClientRef<const T> argDefault(SharedClientRef<const T> object)
  {
     return object;
  }

  template<typename T>
  static void outputToken(std::vector<t_atom>&,T) {}
  
  static void outputToken(std::vector<t_atom>& v,std::shared_ptr<BufferAdaptor> const& b) {
    t_symbol* name = static_cast<PDBufferAdaptor*>(b.get())->name();
    t_atom res;
    SETSYMBOL(&res, name);
    v.push_back(res);
  }

  template <typename T>
  static void outputToken(std::vector<t_atom>& v, SharedClientRef<T> const& ds)
  {
    t_symbol* name = gensym(ds.name());
    t_atom res;
    SETSYMBOL(&res, name);
    v.push_back(res);
  }

  template <typename T>
  static void outputToken(std::vector<t_atom>&, SharedClientRef<const T>)
  {}

  index        mListSize;
  Result       mResult;
  t_outlet*    mNRTProgressOutlet;
  t_outlet*    mControlOutlet;
  bool         mVerbose;
  ParamSetType mParams;
  ParamValues mParamSnapshot;
  Client       mClient;

  bool               mAutosize;
  
  FluidTensor<double, 2>                  mInputListData;
  std::vector<FluidTensorView<double, 1>> mInputListViews;
  FluidTensor<double, 2>                  mOutputListData;
  std::vector<FluidTensorView<double, 1>> mOutputListViews;
  std::vector<t_outlet*>                  mDataOutlets;
  std::vector<t_atom>                     mOutputListAtoms;
  std::vector<InletProxy*>                mProxies;
  std::vector<t_pd*>                      mManagedBuffers;
  
  std::vector<impl::ArrayManager>         mHostedOutputBufferObjects;
  
  std::vector<void(*)(FluidPDWrapper*,int,t_atom*)> mBufferDispatch;
  
  t_canvas*    mCanvas;
  bool         mInitialized{false};
  
  using ArgKey = std::pair<size_t,size_t>;
  std::map<ArgKey,t_symbol*> mArgObjects;
  std::map<ArgKey,impl::ArrayManager> mArgBuffers;
};

////////////////////////////////////////////////////////////////////////////////

template <typename RT = std::true_type>
struct InputTypeWrapper
{
  using type = t_sample;
};

template <>
struct InputTypeWrapper<std::false_type>
{
  using type = float;
};

template <class Client>
void makePDWrapper(const char* classname)
{
  FluidPDWrapper<Client>::makeClass(classname);
}


} // namespace client
} // namespace fluid
