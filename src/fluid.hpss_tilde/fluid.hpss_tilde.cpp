#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ehpss_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<HPSSClient>("fluid.hpss~");
}
