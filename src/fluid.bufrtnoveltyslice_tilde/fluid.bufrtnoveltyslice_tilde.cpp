#include <clients/rt/RTNoveltySlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufrtnoveltyslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTRTNoveltySlice>("fluid.bufrtnoveltyslice~");
}
