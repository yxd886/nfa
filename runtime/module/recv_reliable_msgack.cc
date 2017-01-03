//
#include "recv_reliable_msgack.h"
#include "../actor/coordinator.h"
#include "../actor/base/local_send.h"

void recv_reliable_msgack::customized_init(coordinator* coordinator_actor){
  coordinator_actor_ = coordinator_actor;
}

void recv_reliable_msgack::ProcessBatch(bess::PacketBatch *batch){
  cp_pkt_batch.clear();

  for(int i=0; i<batch->cnt(); i++){
    char* data_start = batch->pkts()[i]->head_data<char*>();

    if(unlikely( ((*((uint16_t*)(data_start+14)) & 0x00f0) != 0x0040) ||
                 ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x00FF))  ){
      coordinator_actor_->gp_collector_.collect(batch->pkts()[i]);
      continue;
    }

    cp_pkt_batch.add(batch->pkts()[i]);
  }

  if(unlikely(cp_pkt_batch.cnt()==0)){
    return;
  }

  send(coordinator_actor_, cp_pkt_batch_t::value, &cp_pkt_batch);

  if(coordinator_actor_->ec_scheduler_batch_.cnt()>0){
    RunSplit(coordinator_actor_->ec_scheduler_gates_,
             &(coordinator_actor_->ec_scheduler_batch_));
  }
}

ADD_MODULE(recv_reliable_msgack, "recv_reliable_msgack",
    "this module handles packets received from the control port")
