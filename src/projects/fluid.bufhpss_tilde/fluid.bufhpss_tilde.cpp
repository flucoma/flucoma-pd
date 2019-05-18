#include <clients/rt/HPSSClient.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTHPSS>("fluid.bufhpss~");
}
