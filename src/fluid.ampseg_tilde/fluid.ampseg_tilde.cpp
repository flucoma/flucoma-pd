#include <clients/rt/AmpSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2eampslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<AmpSlice>("fluid.ampslice~");
}
