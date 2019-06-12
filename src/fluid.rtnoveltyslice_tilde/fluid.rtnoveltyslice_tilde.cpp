#include <clients/rt/RTNoveltySlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidrtnoveltyslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<RTNoveltySlice>("fluidrtnoveltyslice~");
}
