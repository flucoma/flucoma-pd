#include <clients/rt/LoudnessClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufloudness_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTLoudnessClient>("fluid.bufloudness~");
}
