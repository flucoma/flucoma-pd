#include <clients/rt/GainClient.hpp>
#include "FluidPDWrapper.hpp"

extern "C" void fluid0x2egain_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<GainClient>("fluid.gain~");
}
