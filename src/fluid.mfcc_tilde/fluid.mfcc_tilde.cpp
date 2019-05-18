#include <clients/rt/MFCCClient.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<MFCCClient>("fluid.mfcc~");
}
