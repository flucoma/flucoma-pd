#include <clients/rt/TransientSlice.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidtransientslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<TransientsSlice>("fluidtransientslice~");
}
