#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2esines_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<SinesClient>("fluid.sines~");
}
