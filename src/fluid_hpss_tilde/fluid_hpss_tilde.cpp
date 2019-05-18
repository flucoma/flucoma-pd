#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_hpss_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<HPSSClient>("fluid_hpss~");
}
