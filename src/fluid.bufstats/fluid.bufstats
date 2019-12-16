#include <clients/nrt/BufStatsClient.hpp>
#include <FluidPDWrapper.hpp>

extern "C" void setup_fluid0x2ebufstats_tilde(void)
{
  using namespace fluid::client;
  makePDWrapper<NRTThreadedBufStatsClient>("fluid.bufstats");
}
