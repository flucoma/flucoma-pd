#include <clients/rt/GainClient.hpp>
#include "FluidPDWrapper.hpp"

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<GainClient>("fluid.gain~");
}
