#include <clients/rt/OnsetSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_onsetslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<OnsetSlice>("fluid_onsetslice~");
}
