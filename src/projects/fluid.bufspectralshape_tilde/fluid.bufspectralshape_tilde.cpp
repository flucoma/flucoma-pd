#include <clients/rt/SpectralShapeClient.hpp>
#include <FluidPDWrapper.hpp>

void ext_main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTSpectralShapeClient>("fluid.bufspectralshape~");
}
