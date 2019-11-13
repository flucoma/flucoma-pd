#include <clients/rt/TransientSliceClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2etransientslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<TransientsSliceClient>("fluid.transientslice~");
}
