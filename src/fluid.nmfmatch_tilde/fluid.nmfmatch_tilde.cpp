#include <clients/rt/NMFMatch.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NMFMatch>("fluid.nmfmatch~");
}
