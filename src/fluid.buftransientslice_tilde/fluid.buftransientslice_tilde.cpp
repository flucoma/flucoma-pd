#include <clients/rt/TransientSlice.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTTransientSlice>("fluid.buftransientslice~");
}
