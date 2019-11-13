#include <clients/nrt/FluidThreadTestClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufthreaddemo_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedThreadTestClient>("fluid.bufthreaddemo~");
}
