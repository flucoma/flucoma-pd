#include <clients/rt/BaseSTFTClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_stftpass_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<BaseSTFTClient>("fluid_stftpass~");
}
