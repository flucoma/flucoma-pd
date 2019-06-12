#include <clients/rt/MelBandsClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2emelbands_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<MelBandsClient>("fluid.melbands~");
}
