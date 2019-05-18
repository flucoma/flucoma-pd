#include <clients/rt/OnsetSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_bufonsetslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTOnsetSlice>("fluid_bufonsetslice~");
}
