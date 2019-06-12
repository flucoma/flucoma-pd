#include <clients/rt/AmpSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufampslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTAmpSlice>("fluidbufampslice~");
}
