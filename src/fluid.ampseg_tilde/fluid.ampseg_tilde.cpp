#include <clients/rt/AmpSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidampslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<AmpSlice>("fluidampslice~");
}
