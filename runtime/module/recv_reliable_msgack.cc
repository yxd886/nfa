//
#include "recv_reliable_msgack.h"
#include "../actor/coordinator.h"
#include "../actor/base/local_send.h"

void recv_reliable_msgack::customized_init(coordinator* coordinator_actor){
  coordinator_actor_ = coordinator_actor;
}

void recv_reliable_msgack::ProcessBatch(bess::PacketBatch *batch){
  send(coordinator_actor_, control_pkts_batch_t::value, batch);
}
