#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufmfcc_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTMFCCClient>("fluidbufmfcc~");
}
