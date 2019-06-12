#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufpitch_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTPitchClient>("fluidbufpitch~");
}
