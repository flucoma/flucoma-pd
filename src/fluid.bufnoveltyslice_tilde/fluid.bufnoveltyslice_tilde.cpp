#include <clients/nrt/NoveltyClient.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NoveltyClient>("fluid.bufnoveltyslice~");
}
