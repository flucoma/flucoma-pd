#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_bufmfcc_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTMFCCClient>("fluid_bufmfcc~");
}
