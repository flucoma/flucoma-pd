#include <clients/rt/MelBandsClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufmelbands_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTMelBandsClient>("fluidbufmelbands~");
}
