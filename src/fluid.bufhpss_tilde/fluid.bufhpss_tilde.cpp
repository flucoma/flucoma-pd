#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufhpss_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedHPSSClient>("fluid.bufhpss~");
}
