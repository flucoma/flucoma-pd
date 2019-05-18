#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_bufsines_tilde(void)
{
  using namespace fluid::client;
  
  makePDWrapper<NRTSines>("fluid_bufsines~");
}
