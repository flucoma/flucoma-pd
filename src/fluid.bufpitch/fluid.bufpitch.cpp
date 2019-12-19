#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufpitch(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedPitchClient>("fluid.bufpitch");
}
