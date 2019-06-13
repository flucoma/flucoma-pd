#include <clients/rt/TransientSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebuftransientslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTTransientSlice>("fluid.buftransientslice~");
}
