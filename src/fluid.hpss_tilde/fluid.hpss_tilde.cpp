#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<HPSSClient>("fluid.hpss~");
}
