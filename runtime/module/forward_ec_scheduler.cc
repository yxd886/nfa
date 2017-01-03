//
#include "forward_ec_scheduler.h"

#include "../actor/coordinator.h"
#include "../actor/base/local_send.h"

void forward_ec_scheduler::ProcessBatch(bess::PacketBatch *batch){
  send(coordinator_actor_, es_scheduler_pkt_batch_t::value, batch);
  RunNextModule(&(coordinator_actor_->ec_scheduler_batch_));
}

void forward_ec_scheduler::customized_init(coordinator* coordinator_actor){
  coordinator_actor_ = coordinator_actor;
}

ADD_MODULE(forward_ec_scheduler, "forward_ec_scheduler",
    "process packets received from input port to output port and schedule actors in forward direction")
