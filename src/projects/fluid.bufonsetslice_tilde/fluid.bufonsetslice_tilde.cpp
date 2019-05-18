#include <clients/rt/OnsetSlice.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTOnsetSlice>("fluid.bufonsetslice~");
}
