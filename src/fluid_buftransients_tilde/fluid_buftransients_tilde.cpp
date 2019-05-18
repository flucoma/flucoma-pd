#include <clients/rt/TransientClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_buftransients_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTTransients>("fluid_buftransients~");
}
