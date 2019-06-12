#include <clients/rt/BaseSTFTClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidstftpass_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<BaseSTFTClient>("fluidstftpass~");
}
