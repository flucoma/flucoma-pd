#include <clients/nrt/BufferComposeNRT.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufcompose_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<BufferComposeClient>("fluid.bufcompose~");
}
