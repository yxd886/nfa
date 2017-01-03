//
#include "forward_ec_scheduler.h"

#include "../actor/coordinator.h"
#include "../actor/base/local_send.h"

void forward_ec_scheduler::ProcessBatch(bess::PacketBatch *batch){
  dp_pkt_batch.clear();
  cp_pkt_batch.clear();

  for(int i=0; i<batch->cnt(); i++){
    char* data_start = batch->pkts()[i]->head_data<char*>();

    if(unlikely( ((*((uint16_t*)(data_start+14)) & 0x00f0) != 0x0040) ||
                 ( ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x0006) &&
                   ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x0011) &&
                   ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x00FF)) ) ){
      coordinator_actor_->gp_collector_.collect(batch->pkts()[i]);
      continue;
    }

    if(unlikely(((*((uint16_t*)(data_start+23)) & 0x00ff) == 0x00FF))){
      cp_pkt_batch.add(batch->pkts()[i]);
      continue;
    }

    dp_pkt_batch.add(batch->pkts()[i]);
  }

  if(unlikely(cp_pkt_batch.cnt()>0)){
    send(coordinator_actor_, cp_pkt_batch_t::value, &cp_pkt_batch);
  }

  if(unlikely(dp_pkt_batch.cnt()==0)){
    return;
  }

  send(coordinator_actor_, dp_pkt_batch_t::value, &dp_pkt_batch);
  if(unlikely(coordinator_actor_->ec_scheduler_batch_.cnt()==0)){
    return;
  }

  RunNextModule(&(coordinator_actor_->ec_scheduler_batch_));
}

void forward_ec_scheduler::customized_init(coordinator* coordinator_actor){
  coordinator_actor_ = coordinator_actor;
}

ADD_MODULE(forward_ec_scheduler, "forward_ec_scheduler",
    "process packets received from input port to output port and schedule actors in forward direction")
