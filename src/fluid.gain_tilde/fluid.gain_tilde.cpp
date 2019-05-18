#include <clients/rt/GainClient.hpp>
#include "FluidPDWrapper.hpp"

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<GainClient>("fluid.gain~");
}
