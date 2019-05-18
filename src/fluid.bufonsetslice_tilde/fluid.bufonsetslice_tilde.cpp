#include <clients/rt/OnsetSlice.hpp>
#include <FluidPDWrapper.hpp>

void main(void*)
{
  using namespace fluid::client;
  makePDWrapper<NRTOnsetSlice>("fluid.bufonsetslice~");
}
