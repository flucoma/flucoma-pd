#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidmfcc_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<MFCCClient>("fluidmfcc~");
}
