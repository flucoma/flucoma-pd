#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufpitch_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTPitchClient>("fluid.bufpitch~");
}
