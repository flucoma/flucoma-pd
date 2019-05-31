#include <clients/rt/RTNoveltySlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufrtnoveltyslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTRTNoveltySlice>("fluidbufrtnoveltyslice~");
}
