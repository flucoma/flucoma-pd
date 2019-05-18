#include <clients/nrt/NoveltyClient.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NoveltyClient>("fluid.bufnoveltyslice~");
}
