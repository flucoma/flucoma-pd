#include <clients/nrt/NoveltyClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufnoveltyslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NoveltyClient>("fluid.bufnoveltyslice~");
}
