#include <clients/rt/MelBandsClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufmelbands(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedMelBandsClient>("fluid.bufmelbands");
}