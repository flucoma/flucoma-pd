#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidhpss_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<HPSSClient>("fluidhpss~");
}
