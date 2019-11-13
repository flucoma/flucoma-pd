#include <clients/rt/OnsetSliceClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2eonsetslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<OnsetSliceClient>("fluid.onsetslice~");
}
