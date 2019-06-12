#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufhpss_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTHPSS>("fluid.bufhpss~");
}
