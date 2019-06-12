#include <clients/rt/NMFFilter.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2enmffilter_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFFilter>("fluid.nmffilter~");
}
