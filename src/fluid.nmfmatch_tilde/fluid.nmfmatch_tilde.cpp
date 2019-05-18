#include <clients/rt/NMFMatch.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NMFMatch>("fluid.nmfmatch~");
}
