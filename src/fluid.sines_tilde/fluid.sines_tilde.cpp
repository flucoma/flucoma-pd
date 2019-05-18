#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<SinesClient>("fluid.sines~");
}
