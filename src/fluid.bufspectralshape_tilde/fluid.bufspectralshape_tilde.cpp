#include <clients/rt/SpectralShapeClient.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTSpectralShapeClient>("fluid.bufspectralshape~");
}
