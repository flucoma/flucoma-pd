#include <clients/rt/TransientSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_transientslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<TransientsSlice>("fluid_transientslice~");
}
