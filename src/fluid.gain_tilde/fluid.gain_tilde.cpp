#include <clients/rt/GainClient.hpp>
#include "FluidPDWrapper.hpp"

extern "C" void setup_fluid0x2egain_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<GainClient>("fluid.gain~");
}
