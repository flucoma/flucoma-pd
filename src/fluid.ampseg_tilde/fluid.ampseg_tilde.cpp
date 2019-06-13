#include <clients/rt/AmpSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2eampslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<AmpSlice>("fluid.ampslice~");
}
