#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufsines_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedSinesClient>("fluid.bufsines~");
}
