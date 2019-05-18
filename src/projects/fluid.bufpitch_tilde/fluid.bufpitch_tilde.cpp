#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTPitchClient>("fluid.bufpitch~");
}
