#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2emfcc_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<MFCCClient>("fluid.mfcc~");
}
