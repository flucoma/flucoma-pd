#include <clients/nrt/BufferComposeNRT.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufcompose_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<BufferComposeClient>("fluidbufcompose~");
}
