#include <clients/rt/TransientSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebuftransientslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTTransientSlice>("fluid.buftransientslice~");
}
