#include <clients/rt/NMFMatch.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidnmfmatch_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFMatch>("fluidnmfmatch~");
}
