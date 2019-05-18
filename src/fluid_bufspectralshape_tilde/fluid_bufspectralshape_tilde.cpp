#include <clients/rt/SpectralShapeClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_bufspectralshape_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTSpectralShapeClient>("fluid_bufspectralshape~");
}
