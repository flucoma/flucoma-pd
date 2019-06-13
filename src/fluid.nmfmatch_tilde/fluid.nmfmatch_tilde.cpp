#include <clients/rt/NMFMatch.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2enmfmatch_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFMatch>("fluid.nmfmatch~");
}
