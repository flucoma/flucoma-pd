#include <clients/rt/TransientSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbuftransientslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTTransientSlice>("fluidbuftransientslice~");
}
