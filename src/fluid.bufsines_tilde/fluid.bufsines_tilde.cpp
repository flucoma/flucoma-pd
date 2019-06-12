#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufsines_tilde_setup(void)
{
  using namespace fluid::client;
  
  makePDWrapper<NRTSines>("fluid.bufsines~");
}
