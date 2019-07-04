#include <clients/rt/BaseSTFTClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2estftpass_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<BaseSTFTClient>("fluid.stftpass~");
}
