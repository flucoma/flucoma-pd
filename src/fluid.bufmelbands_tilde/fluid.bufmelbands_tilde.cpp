#include <clients/rt/MelBandsClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufmelbands_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTMelBandsClient>("fluid.bufmelbands~");
}
