#include <clients/rt/TransientClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2etransients_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<TransientClient>("fluid.transients~");
}
