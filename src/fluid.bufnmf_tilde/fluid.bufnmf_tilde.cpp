#include <clients/nrt/NMFClient.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NMFClient>("fluid.bufnmf~");
}


