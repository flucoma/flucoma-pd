#include <clients/rt/AmpSliceClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufampslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedAmpSliceClient>("fluid.bufampslice~");
}
