#include <clients/rt/NMFFilter.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_nmffilter_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFFilter>("fluid_nmffilter~");
}
