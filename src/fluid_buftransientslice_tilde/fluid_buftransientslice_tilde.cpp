#include <clients/rt/TransientSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_buftransientslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTTransientSlice>("fluid_buftransientslice~");
}
