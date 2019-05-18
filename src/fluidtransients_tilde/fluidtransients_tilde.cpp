#include <clients/rt/TransientClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidtransients_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<TransientClient>("fluidtransients~");
}
