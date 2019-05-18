#include <clients/nrt/BufferComposeNRT.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<BufferComposeClient>("fluid.bufcompose~");
}
