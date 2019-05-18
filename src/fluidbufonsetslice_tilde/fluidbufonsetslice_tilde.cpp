#include <clients/rt/OnsetSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufonsetslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTOnsetSlice>("fluidbufonsetslice~");
}
