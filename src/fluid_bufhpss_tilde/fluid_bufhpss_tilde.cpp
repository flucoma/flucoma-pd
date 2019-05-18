#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_bufhpss_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTHPSS>("fluid_bufhpss~");
}
