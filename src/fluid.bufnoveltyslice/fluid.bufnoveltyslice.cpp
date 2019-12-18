#include <clients/rt/NoveltySliceClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufnoveltyslice(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadingNoveltySliceClient>("fluid.bufnoveltyslice");
}
