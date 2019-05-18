#include <clients/rt/SpectralShapeClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluidbufspectralshape_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTSpectralShapeClient>("fluidbufspectralshape~");
}
