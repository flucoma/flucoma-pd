#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTMFCCClient>("fluid.bufmfcc~");
}
