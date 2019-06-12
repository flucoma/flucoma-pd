#include <clients/rt/BaseSTFTClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2estftpass_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<BaseSTFTClient>("fluid.stftpass~");
}
