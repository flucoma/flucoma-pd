#include <clients/rt/MelBandsClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidmelbands_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<MelBandsClient>("fluidmelbands~");
}
