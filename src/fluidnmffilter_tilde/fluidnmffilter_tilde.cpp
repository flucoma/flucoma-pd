#include <clients/rt/NMFFilter.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidnmffilter_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFFilter>("fluidnmffilter~");
}
