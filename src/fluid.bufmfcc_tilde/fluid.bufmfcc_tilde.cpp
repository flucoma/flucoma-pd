#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufmfcc_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTMFCCClient>("fluid.bufmfcc~");
}
