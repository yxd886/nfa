//
#include "ec_scheduler.h"
#include "../actor/coordinator.h"
#include "../actor/base/local_send.h"

pb_error_t ec_scheduler::Init(const bess::pb::PortIncArg &arg){
  return pb_errno(0);
}

void ec_scheduler::ProcessBatch(bess::PacketBatch *batch){
  send(coordinator_actor_, es_scheduler_pkt_batch_t::value, batch);

  RunSplit(coordinator_actor_->peek_ec_scheduler_gates(),
           coordinator_actor_->peek_ec_scheduler_batch());
}

void ec_scheduler::customized_init(coordinator* coordinator_actor){
  coordinator_actor_ = coordinator_actor;
}

ADD_MODULE(ec_scheduler, "ec_scheduler", "schedule flow actors")
