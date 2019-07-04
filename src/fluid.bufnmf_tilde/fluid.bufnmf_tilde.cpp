#include <clients/nrt/NMFClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufnmf_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFClient>("fluid.bufnmf~");
}


