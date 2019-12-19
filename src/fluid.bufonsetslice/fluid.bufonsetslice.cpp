#include <clients/rt/OnsetSliceClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufonsetslice(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadingOnsetSliceClient>("fluid.bufonsetslice");
}
