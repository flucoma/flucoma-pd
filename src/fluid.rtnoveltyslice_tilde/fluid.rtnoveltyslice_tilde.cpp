#include <clients/rt/RTNoveltySlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ertnoveltyslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<RTNoveltySlice>("fluid.rtnoveltyslice~");
}
