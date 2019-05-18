#include <clients/rt/GainClient.hpp>
#include "FluidPDWrapper.hpp"

extern "C" void setup_fluid_gain_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<GainClient>("fluid_gain~");
}
