#include <clients/nrt/NMFClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2ebufnmf_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFClient>("fluid.bufnmf~");
}


