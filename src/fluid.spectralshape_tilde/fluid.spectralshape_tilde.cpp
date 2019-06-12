#include <clients/rt/SpectralShapeClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void fluid0x2espectralshape_tilde_setup(void)
{
  using namespace fluid::client;
  makePDWrapper<SpectralShapeClient>("fluid.spectralshape~");
}
