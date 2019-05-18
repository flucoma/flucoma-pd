#include <clients/rt/NMFFilter.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NMFFilter>("fluid.nmffilter~");
}
