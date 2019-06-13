#include <clients/rt/TransientClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2etransients_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<TransientClient>("fluid.transients~");
}
