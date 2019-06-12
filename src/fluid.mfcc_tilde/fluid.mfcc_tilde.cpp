#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2emfcc_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<MFCCClient>("fluid.mfcc~");
}
