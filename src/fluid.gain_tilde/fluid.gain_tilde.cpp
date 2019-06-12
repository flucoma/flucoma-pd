#include <clients/rt/GainClient.hpp>
#include "FluidPDWrapper.hpp"

extern "C" void fluidgain_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<GainClient>("fluidgain~");
}
