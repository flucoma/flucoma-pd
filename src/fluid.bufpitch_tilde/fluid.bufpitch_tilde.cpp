#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTPitchClient>("fluid.bufpitch~");
}
