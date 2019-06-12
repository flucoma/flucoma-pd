#include <clients/rt/TransientClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebuftransients_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTTransients>("fluid.buftransients~");
}
