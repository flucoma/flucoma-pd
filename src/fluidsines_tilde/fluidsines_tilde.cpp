#include <clients/rt/SinesClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidsines_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<SinesClient>("fluidsines~");
}
