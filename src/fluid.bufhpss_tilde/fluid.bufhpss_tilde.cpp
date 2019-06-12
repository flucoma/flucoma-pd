#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufhpss_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTHPSS>("fluidbufhpss~");
}
