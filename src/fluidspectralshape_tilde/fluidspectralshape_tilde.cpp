#include <clients/rt/SpectralShapeClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidspectralshape_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<SpectralShapeClient>("fluidspectralshape~");
}
