#include <clients/rt/BaseSTFTClient.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<BaseSTFTClient>("fluid.stftpass~");
}
