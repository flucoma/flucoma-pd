#pragma once

#include "m_pd.h"

#include <clients/common/FluidBaseClient.hpp>
#include <clients/common/OfflineClient.hpp>
#include <clients/common/ParameterSet.hpp>
#include <clients/common/ParameterTypes.hpp>

#include <PDBufferAdaptor.hpp>

#include <tuple>
#include <utility>
#include <cstdarg>
#include <cctype>
#include <cstring>

namespace fluid {
namespace client {

//////////////////////////////////////////////////////////////////////////////////////////////////

namespace impl {
    
template <class T>
void object_warn(T *x, const char *fmt, ...)
{
  char postString[1024];
  char finalString[1024];
  
  char *objectName = class_getname(*((t_pd *)x));
  va_list argp;
  va_start(argp, fmt);
  vsnprintf(postString, 1024, fmt, argp);
  va_end(argp);
  snprintf(finalString, 1024, "%s: %s", objectName, finalString);
 
  post(fmt, finalString);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

template <class Wrapper>
class RealTime
{
  using ViewType = FluidTensorView<t_sample, 1>;

public:

  //Make sure we free the clock if it's in use, otherwise badness
  ~RealTime() { if(mControlClock) clock_free(mControlClock); }

  static void setup(t_class *c)
  {
    class_addmethod(c, nullfn, gensym("signal"), A_NULL);
    class_addmethod(c, (t_method) callDSP, gensym("dsp"), A_CANT, 0);

    class_addmethod(c, (t_method) getLatency, gensym("getlatency"), A_NULL);
  }

  static void getLatency(Wrapper *x)
  {
    outlet_float(x->mLatencyOut, x->mClient.latency());
  }
    
  static void callDSP(Wrapper *x, t_signal **sp)
  {
    x->dsp(sp);
  }

  static t_int *callPerform(t_int *w)
  {
    Wrapper *x = (Wrapper *) w[1];
    int sampleframes = (int) w[2];
    x->perform(sampleframes);
    return w + 3;
  }

  void setupAudio(t_object *pdObject, size_t numSigIns, size_t numSigOuts)
  {
    Wrapper *wrapper = static_cast<Wrapper *>(this);
    auto &   client  = wrapper->client();
      
    //impl::PDBase::getPDObject()->z_misc |= Z_NO_INPLACE;
          
    mSigIns.resize(numSigIns);
    mSigOuts.resize(numSigOuts);
          
    // Create signal inlets
          
    for (size_t i = 0; numSigIns && i < (numSigIns - 1); i++)
        signalinlet_new(pdObject, 0.0);
          
    // Create signal outlets
          
    for (size_t i = 0; i < numSigOuts; i++)
      outlet_new(pdObject, gensym("signal"));
      
    if (client.controlChannelsOut())
        wrapper->mControlOutlet = outlet_new(pdObject, gensym("list"));
      
    // Create latency ouput
      
    mLatencyOut = outlet_new(pdObject, gensym("float"));
  }
    
  void dsp(t_signal **sp)
  {
    Wrapper *wrapper = static_cast<Wrapper *>(this);
    
    wrapper->mClient = typename Wrapper::ClientType{wrapper->mParams};
    auto &   client  = wrapper->client();
    client.sampleRate(sp[0]->s_sr);

    assert((client.audioChannelsOut() > 0) != (client.controlChannelsOut() > 0) &&
           "Client must *either* be audio out or control out, sorry");

    mInputs = std::vector<ViewType>(client.audioChannelsIn(), ViewType(nullptr, 0, 0));

    if (client.audioChannelsOut() > 0) mOutputs = std::vector<ViewType>(client.audioChannelsOut(), ViewType(nullptr, 0, 0));
    if (client.controlChannelsOut() > 0)
    {
      mControlClock = mControlClock ? mControlClock : clock_new((t_object *) wrapper, (t_method) doControlOut);

      mOutputs = std::vector<ViewType>(client.controlChannelsOut(), ViewType(nullptr, 0, 0));
      mControlOutputs.resize(client.controlChannelsOut());
      mControlAtoms.resize(client.controlChannelsOut());
    }

    for (size_t i = 0; i < mSigIns.size(); i++)
      mSigIns[i] = sp[i]->s_vec;
      
    for (size_t i = 0; i < mSigOuts.size(); i++)
      mSigOuts[i] = sp[i + mSigIns.size()]->s_vec;
      
    dsp_add(callPerform, 2, wrapper, sp[0]->s_vecsize);
  }

  void perform(int sampleframes)
  {
    auto &client = static_cast<Wrapper *>(this)->mClient;
    for (auto i = 0u; i < mSigIns.size(); ++i)
      mInputs[i].reset(mSigIns[i], 0, sampleframes);

    for (auto i = 0u; i < client.audioChannelsOut(); ++i)
      mOutputs[i].reset(mSigOuts[i], 0, sampleframes);

    for (auto i = 0u; i < client.controlChannelsOut(); ++i) mOutputs[i].reset(&mControlOutputs[i], 0, 1);

    client.process(mInputs, mOutputs, mContext);

    if (mControlClock)
      clock_delay(mControlClock, 0);
  }

  void controlData()
  {
    Wrapper *w      = static_cast<Wrapper *>(this);
    auto &   client = w->client();
      
    for (size_t i = 0; i < client.controlChannelsOut(); i++)
      SETFLOAT(mControlAtoms.data() + i, mControlOutputs[i]);

    w->controlOut(static_cast<int>(client.controlChannelsOut()), mControlAtoms.data());
  }

private:
  static void               doControlOut(Wrapper *x) { x->controlData(); }
  std::vector<ViewType>     mInputs;
  std::vector<ViewType>     mOutputs;
  std::vector<t_sample *>   mSigIns;
  std::vector<t_sample *>   mSigOuts;
  std::vector<t_float>      mControlOutputs;
  std::vector<t_atom>       mControlAtoms;
  t_outlet*                 mLatencyOut;
  t_clock*                  mControlClock{nullptr};
  FluidContext              mContext;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

template <class Wrapper>
struct NonRealTime
{
  
  NonRealTime()
  {
  
    auto w = static_cast<Wrapper*>(this);
  
    mProgressClock = clock_new((t_object*) w  , (t_method)checkProcess);
  }
  
  ~NonRealTime()
  {
    if(mProgressClock) clock_free(mProgressClock);
  }
  
  static void setup(t_class *c)
  {
    class_addmethod(c, (t_method) callProcess, gensym("bang"), A_NULL);
    class_addmethod(c, (t_method) callSR, gensym("sr"), A_FLOAT, 0);
    
    //concurrency messages
    class_addmethod(c, (t_method) callCancel, gensym("cancel"), A_NULL);
    class_addmethod(c, (t_method) doSynchronous, gensym("synchronous"), A_FLOAT,0);
  }

  bool checkResult(Result& res)
  {
    auto &wrapper = static_cast<Wrapper &>(*this);
    
    if (!res.ok())
    {
      Wrapper::printResult(&wrapper,res);
      return false;
    }
    return true;
  }

  void cancel()
  {
    auto &wrapper = static_cast<Wrapper &>(*this);
    auto &client  = wrapper.mClient;
    client.cancel();
  }
  
  void process()
  {
    auto &wrapper = static_cast<Wrapper &>(*this);
    auto &client  = wrapper.mClient;
    bool synchronous = mSynchronous;
    
    client.setSynchronous(synchronous);
    
    Result res = client.process();
    if (checkResult(res))
    {
      if (synchronous)
        wrapper.doneBang();
      else
        clockWait();
    }
  }
  static void callCancel(Wrapper *x) { x->cancel(); }
  static void callProcess(Wrapper *x) { x->process(); }
  
  static void checkProcess(Wrapper *x)
  {
    Result res;
    auto &client  = x->mClient;
    
    if (client.checkProgress(res) == ProcessState::kDone)
    {
      if (x->checkResult(res))
        x->doneBang();
    }
    else
    {
      x->progress(client.progress());
      x->clockWait();
    }
  }
  
  void clockWait()
  {
    clock_delay(mProgressClock, 20);  // FIX - set at 20ms for now...
  }
  
  void setupAudio(t_object *, size_t, size_t) {}
  
  static void callSR(Wrapper *x, t_float sr) { x->sampleRate(sr); }
  
  static void doSynchronous(Wrapper *x, t_float s) { x->mSynchronous = static_cast<bool>(s);   }
  
private:
  t_clock* mProgressClock;
  bool     mSynchronous;
  
};

//////////////////////////////////////////////////////////////////////////////////////////////////

template <class Wrapper>
struct NonRealTimeAndRealTime : public RealTime<Wrapper>, public NonRealTime<Wrapper>
{
  static void setup(t_class *c)
  {
    RealTime<Wrapper>::setup(c);
    NonRealTime<Wrapper>::setup(c);
  }
    
  void setupAudio(t_object *pdObject, size_t numSigIns, size_t numSigOuts)
  {
    RealTime<Wrapper>::setupAudio(pdObject, numSigIns, numSigOuts);
    NonRealTime<Wrapper>::setupAudio(pdObject, numSigIns, numSigOuts);
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////////

// Max base type

struct PDBase
{
  t_object*   getPDObject() { return &mObject; }
    
  t_object    mObject;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

// Templates and specialisations for all possible base options

template <class Wrapper, typename NRT, typename RT>
struct FluidPDBase : public PDBase
{
  static_assert(isRealTime<FluidPDBase>::value || isNonRealTime<FluidPDBase>::value,
                "This object seems to be neither real-time nor non-real-time! Check that your Client inherits from "
                "Audio[In/Out], Control[In/Out] or Offline[In/Out]");
};

template <class Wrapper>
struct FluidPDBase<Wrapper, std::true_type, std::false_type> : public PDBase, public NonRealTime<Wrapper>
{};

template <class Wrapper>
struct FluidPDBase<Wrapper, std::false_type, std::true_type> : public PDBase, public RealTime<Wrapper>
{};

template <class Wrapper>
struct FluidPDBase<Wrapper, std::true_type, std::true_type> : public PDBase, public NonRealTimeAndRealTime<Wrapper>
{};

//////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace impl

template <typename Client>
class FluidPDWrapper : public impl::FluidPDBase<FluidPDWrapper<Client>, isNonRealTime<Client>, isRealTime<Client>>
{
  using WrapperBase = impl::FluidPDBase<FluidPDWrapper<Client>, isNonRealTime<Client>, isRealTime<Client>>;

  friend impl::RealTime<FluidPDWrapper<Client>>;
  friend impl::NonRealTime<FluidPDWrapper<Client>>;

  template <size_t N>
  static constexpr auto paramDescriptor() { return Client::getParameterDescriptors().template get<N>(); }

  static void printResult(FluidPDWrapper<Client>* x, Result& r)
  {
    if (!x) return;

    if (x->verbose() && !x->messages().ok())
    {
      switch (x->messages().status())
      {
        case Result::Status::kWarning: object_warn(x, r.message().c_str()); break;
        case Result::Status::kError: pd_error(x, "%s", r.message().c_str()); break;
        case Result::Status::kCancelled: object_warn(x, "Job cancelled"); break;
        default: {
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  template <size_t N, typename T, typename U, U Method(t_atom *av)>
  struct FetchValue
  {
    typename T::type operator()(const long ac, t_atom *av, long &currentCount)
    {
      auto defaultValue = paramDescriptor<N>().defaultValue;
      return currentCount < ac ? Method(av + currentCount++) : defaultValue;
    }
  };

  template <size_t N, typename T>
  struct Fetcher;

  template <size_t N>
  struct Fetcher<N, FloatT> : public FetchValue<N, FloatT, t_float, atom_getfloat>
  {};

  template <size_t N>
  struct Fetcher<N, LongT> : public FetchValue<N, LongT, t_int, atom_getint>
  {};

  //////////////////////////////////////////////////////////////////////////////////////////////////

  // Setter

  template<typename T, size_t N>
  struct Setter
  {
    static constexpr size_t argSize = paramDescriptor<N>().fixedSize;

    static auto fromAtom(FluidPDWrapper<Client>*, t_atom *a, LongT::type) { return atom_getint(a); }
    static auto fromAtom(FluidPDWrapper<Client>*, t_atom *a, FloatT::type) { return atom_getfloat(a); }

    static auto fromAtom(FluidPDWrapper<Client>* x, t_atom *a, BufferT::type)
    {
      return BufferT::type(new PDBufferAdaptor(atom_getsymbol(a), x->sampleRate()));
    }

    static void set(FluidPDWrapper<Client>* x, t_symbol *s, int ac, t_atom *av)
    {
      NOTUSED(s);
        
      ParamLiteralConvertor<T, argSize> a;
      a.set(paramDescriptor<N>().defaultValue);

      x->messages().reset();

      for (auto i = 0u; i < argSize && i < static_cast<size_t>(ac); i++)
        a[i] = fromAtom(x, av + i, a[0]);

      x->params().template set<N>(a.value(), x->verbose() ? &x->messages() : nullptr);
      printResult(x, x->messages());
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////

  // Getter

  template<typename T, size_t N>
  struct Getter
  {
    static constexpr size_t argSize = paramDescriptor<N>().fixedSize;

    static auto toAtom(t_atom *a, LongT::type v) { SETFLOAT(a, v); }
    static auto toAtom(t_atom *a, FloatT::type v) { SETFLOAT(a, static_cast<t_float>(v)); }

    static auto toAtom(t_atom *a, BufferT::type v)
    {
      auto b = static_cast<PDBufferAdaptor *>(v.get());
      SETSYMBOL(a, (b ? b->name() : nullptr));
    }

    static std::vector<t_atom> get(FluidPDWrapper<Client>* x)
    {
      ParamLiteralConvertor<T, argSize> a;
      std::vector<t_atom> result(argSize);
      
      a.set(x->params().template get<N>());

      for (auto i = 0u; i < argSize; i++)
        toAtom(result.data() + i, a[i]);
        
      return result;
    }
  };

public:

  using ClientType    = Client;
  using ParamDescType = typename Client::ParamDescType;
  using ParamSetType = typename Client::ParamSetType;

  FluidPDWrapper(t_symbol*, int ac, t_atom *av)
    : mNRTProgressOutlet{nullptr},mNRTDoneOutlet(nullptr), mControlOutlet(nullptr), mParams(Client::getParameterDescriptors()),
      mParamSnapshot(Client::getParameterDescriptors()),
      mClient{initParamsFromArgs(ac,av)}
  {
    t_object* pdObject = impl::PDBase::getPDObject();
      
    auto results = mParams.keepConstrained(true);
    mParamSnapshot = mParams;

    for (auto &r : results)
      printResult(this, r);

    this->setupAudio(pdObject, mClient.audioChannelsIn(), mClient.audioChannelsOut());

    if (isNonRealTime<Client>::value)
    {      
        mNRTDoneOutlet = outlet_new(pdObject, gensym("bang"));
        mNRTProgressOutlet = outlet_new(pdObject,gensym("float"));
    }

    if (mClient.controlChannelsOut() && !mControlOutlet) mControlOutlet = outlet_new(pdObject, gensym("list"));
  }

  void doneBang() { outlet_bang(mNRTDoneOutlet); }

  void controlOut(int ac, t_atom *av) { outlet_list(mControlOutlet, nullptr, static_cast<short>(ac), av); }

  static bool isTag(t_atom *a)
  {
    t_symbol* s = atom_getsymbol(a);
      
    return a->a_type == A_SYMBOL && s && strlen(s->s_name) > 1 && s->s_name[0] == '-';
  }
    
  static int findTag(int start, int ac, t_atom *av)
  {
    for (int i = start; i < ac; i++)
      if (isTag(av + i)) return i;
      
    return ac;
  }
    
  static int paramArgOffset(int ac, t_atom *av)
  {
    return findTag(0, ac, av);
  }
    
  template <size_t N, typename T>
  struct MatchName
  {
    void operator()(const typename T::type& param, const char *name, bool& matched)
    {
      NOTUSED(param);
        
      std::string paramName = lowerCase(paramDescriptor<N>().name);
        
      if (!strcmp(paramName.c_str(), name))
        matched = true;
    }
  };
    
  void paramArgProcess(int ac, t_atom *av)
  {
    int tag = paramArgOffset(ac, av);
      
    while (tag < ac)
    {
      t_symbol* s = atom_getsymbol(av + tag);
      int endTag = findTag(tag + 1, ac, av);
      
      bool matched = false;
      mParams.template forEachParam<MatchName>(s->s_name + 1, matched);
      if (!strcmp(s->s_name + 1, "warnings"))
        matched = true;
      
      if (isNonRealTime<Client>::value && !strcmp(s->s_name + 1, "synchronous"))
        matched = true;
      
      if (!matched)
      {
        pd_error(impl::PDBase::getPDObject(), "No parameter named %s", s->s_name + 1);
      }
      else
      {
          pd_typedmess((t_pd *) impl::PDBase::getPDObject(), gensym(s->s_name + 1), endTag - (tag + 1), av + tag + 1);
      }
        
      tag = endTag;
    }
  }
    
  static void *create(t_symbol *sym, int ac, t_atom *av)
  {
    void *x = pd_new(getClass());
    new (x) FluidPDWrapper(sym, ac, av);

    if (static_cast<size_t>(paramArgOffset(ac, av)) > ParamDescType::NumFixedParams)
    { impl::object_warn(x, "Too many arguments. Got %d, expect at most %d", ac, ParamDescType::NumFixedParams); }

    return x;
  }

  static void destroy(FluidPDWrapper * x)
  {
    x->~FluidPDWrapper();
  }

  static void makeClass(const char *className)
  {
    const ParamDescType& p = Client::getParameterDescriptors();
    getClass(class_new(gensym(className), (t_newmethod)create, (t_method)destroy, sizeof(FluidPDWrapper), 0, A_GIMME, 0));
    WrapperBase::setup(getClass());

    class_addmethod(getClass(), (t_method)doReset, gensym("reset"), A_NULL);
    class_addmethod(getClass(), (t_method)doWarnings, gensym("warnings"), A_FLOAT, 0);

    p.template iterateMutable<SetupParameter>();
    class_sethelpsymbol(getClass(), gensym(className));
  }

  static void doReset(FluidPDWrapper *x)
  {
    x->mParams = x->mParamSnapshot; 
  }
    
  static void doWarnings(FluidPDWrapper *x, t_float warnings)
  {
    x->mVerbose = (bool) warnings;
  }

  Result &messages() { return mResult; }
  bool    verbose() { return mVerbose; }
  Client &client() { return mClient; }
  ParamSetType &params() { return mParams; }


  void progress(double progress) { outlet_float(mNRTProgressOutlet, static_cast<float>(progress)); }

  void sampleRate(float sr)
  {
      mSamplerate = sr > 0 ? sr : mSamplerate;
      mParams.template forEachParamType<BufferT,SetBufferSR>(this);
  }
  
  float sampleRate() { return mSamplerate <= 0 ? sys_getsr() : mSamplerate; }
private:

  static t_class *getClass(t_class *setClass = nullptr)
  {
    static t_class *c = nullptr;
    return (c = setClass ? setClass : c);
  }

  static std::string lowerCase(const char *str)
  {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
  }

  ParamSetType &initParamsFromArgs(int ac, t_atom *av)
  {
    // Process arguments for instantiation parameters
    if (long numArgs = paramArgOffset(ac, av))
    {
      long argCount{0};
      mParams.template setFixedParameterValues<Fetcher>(true, numArgs, av, argCount);
    }
    // process in-box attributes for mutable params
    paramArgProcess(ac, av);
    // return params so this can be called in client initaliser    
    return mParams;
  }

  // Sets up a single parameter

  template <size_t N, typename T>
  struct SetupParameter
  {
    void operator()(const T &parameter)
    {
      std::string       name            = lowerCase(parameter.name);
      t_method          setterMethod    = (t_method) &Setter<T, N>::set;
        
      class_addmethod(getClass(), setterMethod, gensym(name.c_str()), A_GIMME, 0);
    }
  };
  
  //Set the sample rate of a PD buffer adaptor param
  template <size_t N, typename T>
  struct SetBufferSR
  {
    void operator()(const typename T::type& param, FluidPDWrapper* x)
    {
      if(auto b = static_cast<PDBufferAdaptor*>(param.get())) b->sampleRate(x->sampleRate());
    }
  };
  
  Result        mResult;
  t_outlet*     mNRTProgressOutlet;
  t_outlet*     mNRTDoneOutlet;
  t_outlet*     mControlOutlet;
  bool          mVerbose;
  ParamSetType  mParams;
  ParamSetType  mParamSnapshot;
  Client        mClient;
  float         mSamplerate{-1};
};

//////////////////////////////////////////////////////////////////////////////////////////////////

template<typename RT = std::true_type>
struct InputTypeWrapper
{
  using type = t_sample;
};

template<>
struct InputTypeWrapper<std::false_type>
{
  using type = float;
};

template <template <typename T> class Client>
void makePDWrapper(const char *classname)
{
  using InputType = typename InputTypeWrapper<isRealTime<Client<double>>>::type;

  FluidPDWrapper<Client<InputType>>::makeClass(classname);
}

} // namespace client
} // namespace fluid
