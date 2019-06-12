#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidpitch_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<PitchClient>("fluidpitch~");
}
