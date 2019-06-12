#include <clients/rt/OnsetSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufonsetslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTOnsetSlice>("fluid.bufonsetslice~");
}
