#include <clients/rt/NoveltySliceClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2enoveltyslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NoveltySliceClient>("fluid.noveltyslice~");
}
