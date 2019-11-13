#include <clients/rt/NMFFilterClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2enmffilter_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFFilterClient>("fluid.nmffilter~");
}
