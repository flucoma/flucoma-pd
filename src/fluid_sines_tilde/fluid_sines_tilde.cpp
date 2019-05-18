#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_sines_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<SinesClient>("fluid_sines~");
}
