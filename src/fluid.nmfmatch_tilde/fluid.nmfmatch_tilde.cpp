#include <clients/rt/NMFMatchClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2enmfmatch_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFMatchClient>("fluid.nmfmatch~");
}
