#include <clients/rt/SpectralShapeClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid_spectralshape_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<SpectralShapeClient>("fluid_spectralshape~");
}
