#include <clients/rt/SpectralShapeClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2espectralshape_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<SpectralShapeClient>("fluid.spectralshape~");
}
