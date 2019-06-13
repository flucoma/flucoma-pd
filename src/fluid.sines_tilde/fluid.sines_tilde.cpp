#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2esines_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<SinesClient>("fluid.sines~");
}
