#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_mfcc_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<MFCCClient>("fluid_mfcc~");
}
