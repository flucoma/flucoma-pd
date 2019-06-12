#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ehpss_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<HPSSClient>("fluid.hpss~");
}
