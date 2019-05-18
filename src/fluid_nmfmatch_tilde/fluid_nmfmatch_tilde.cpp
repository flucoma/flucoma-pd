#include <clients/rt/NMFMatch.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_nmfmatch_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFMatch>("fluid_nmfmatch~");
}
