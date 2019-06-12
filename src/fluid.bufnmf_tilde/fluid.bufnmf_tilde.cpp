#include <clients/nrt/NMFClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufnmf_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NMFClient>("fluidbufnmf~");
}


