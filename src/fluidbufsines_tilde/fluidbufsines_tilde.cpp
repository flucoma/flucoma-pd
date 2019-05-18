#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufsines_tilde_setup(void)
{
  using namespace fluid::client;
  
  makePDWrapper<NRTSines>("fluidbufsines~");
}
