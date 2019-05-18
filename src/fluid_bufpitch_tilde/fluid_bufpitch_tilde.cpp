#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_bufpitch_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTPitchClient>("fluid_bufpitch~");
}
