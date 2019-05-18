#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_pitch_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<PitchClient>("fluid_pitch~");
}
