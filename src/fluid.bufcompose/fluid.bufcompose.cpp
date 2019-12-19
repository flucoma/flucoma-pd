#include <clients/nrt/BufComposeClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufcompose(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedBufComposeClient>("fluid.bufcompose");
}
