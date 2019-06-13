#include <clients/rt/RTNoveltySlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufrtnoveltyslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTRTNoveltySlice>("fluid.bufrtnoveltyslice~");
}
