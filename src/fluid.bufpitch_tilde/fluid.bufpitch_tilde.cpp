#include <clients/rt/PitchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufpitch_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTPitchClient>("fluid.bufpitch~");
}
