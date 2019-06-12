#include <clients/rt/NMFMatch.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2enmfmatch_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFMatch>("fluid.nmfmatch~");
}
