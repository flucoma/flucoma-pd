#include <clients/rt/AmpSliceClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2eampslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<AmpSliceClient>("fluid.ampslice~");
}
