#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<SinesClient>("fluid.sines~");
}
