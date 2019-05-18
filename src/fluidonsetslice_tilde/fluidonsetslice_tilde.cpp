#include <clients/rt/OnsetSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidonsetslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<OnsetSlice>("fluidonsetslice~");
}
