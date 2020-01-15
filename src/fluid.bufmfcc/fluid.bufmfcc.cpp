#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufmfcc(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedMFCCClient>("fluid.bufmfcc");
}