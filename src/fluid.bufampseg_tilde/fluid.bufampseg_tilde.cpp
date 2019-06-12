#include <clients/rt/AmpSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufampslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTAmpSlice>("fluid.bufampslice~");
}
