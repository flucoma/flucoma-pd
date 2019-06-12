#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2epitch_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<PitchClient>("fluid.pitch~");
}
