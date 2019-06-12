#include <clients/rt/LoudnessClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufloudness_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTLoudnessClient>("fluidbufloudness~");
}
