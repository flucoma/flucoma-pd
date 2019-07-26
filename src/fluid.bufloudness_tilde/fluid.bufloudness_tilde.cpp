#include <clients/rt/LoudnessClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufloudness_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedLoudnessClient>("fluid.bufloudness~");
}
