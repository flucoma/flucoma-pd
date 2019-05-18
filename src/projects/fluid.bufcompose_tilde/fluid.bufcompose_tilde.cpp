#include <clients/nrt/BufferComposeNRT.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<BufferComposeClient>("fluid.bufcompose~");
}
