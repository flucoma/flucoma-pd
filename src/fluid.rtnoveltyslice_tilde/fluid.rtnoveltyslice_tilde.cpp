#include <clients/rt/RTNoveltySlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ertnoveltyslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<RTNoveltySlice>("fluid.rtnoveltyslice~");
}
