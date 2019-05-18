#include <clients/rt/NMFFilter.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NMFFilter>("fluid.nmffilter~");
}
