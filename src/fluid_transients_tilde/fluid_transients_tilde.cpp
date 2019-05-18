#include <clients/rt/TransientClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_transients_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<TransientClient>("fluid_transients~");
}
