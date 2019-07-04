#include <clients/nrt/BufferComposeNRT.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufcompose_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<BufferComposeClient>("fluid.bufcompose~");
}
