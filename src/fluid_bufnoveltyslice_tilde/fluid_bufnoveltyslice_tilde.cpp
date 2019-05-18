#include <clients/nrt/NoveltyClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_bufnoveltyslice_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NoveltyClient>("fluid_bufnoveltyslice~");
}
