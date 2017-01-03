//
#include "send_reliable_ack.h"

#include <glog/logging.h>

void send_reliable_ack::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
}

struct task_result send_reliable_ack::RunTask(void *arg){
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  bess::PacketBatch batch;
  batch.clear();
  uint16_t out_gates[bess::PacketBatch::kMaxBurst];

  for(auto it=coordinator_actor_->reliables_.begin(); it!=coordinator_actor_->reliables_.end(); it++){
    bess::Packet* ack_pkt = it->second.get_ack_pkt();
    if(unlikely(ack_pkt == nullptr)){
      continue;
    }
    out_gates[batch.cnt()] = it->second.get_output_gate();
    batch.add(ack_pkt);
  }

  if(batch.cnt()>0){
    RunSplit(out_gates, &batch);
  }

  return ret;
}

ADD_MODULE(send_reliable_ack, "send_reliable_ack", "send out all the ack packets")
