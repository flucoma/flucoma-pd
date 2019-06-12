#include <clients/nrt/NoveltyClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufnoveltyslice_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NoveltyClient>("fluidbufnoveltyslice~");
}
