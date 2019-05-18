#include <clients/nrt/BufferComposeNRT.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_bufcompose_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<BufferComposeClient>("fluid_bufcompose~");
}
