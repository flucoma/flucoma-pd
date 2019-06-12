#include <clients/rt/LoudnessClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidloudness_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<LoudnessClient>("fluidloudness~");
}
