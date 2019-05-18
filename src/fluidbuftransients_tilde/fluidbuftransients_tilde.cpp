#include <clients/rt/TransientClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbuftransients_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTTransients>("fluidbuftransients~");
}
