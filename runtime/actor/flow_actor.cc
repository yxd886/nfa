#include "flow_actor.h"
#include "coordinator.h"

void flow_actor::handle_message(pkt_msg_t, bess::Packet* pkt){
  pkt_counter+=1;

  // output phase, ogate 0 of ec_scheduler is connected to the output port.
  // ogate 1 of ec_scheduler is connected to a sink

  int next_available_pos = ec_scheduler_batch_->cnt();
  ec_scheduler_gates_[next_available_pos] = 0;
  ec_scheduler_batch_->add(pkt);
}

void flow_actor::handle_message(ec_scheduler_batch_and_gates_t,
                                bess::PacketBatch* ec_scheduler_batch,
                                gate_idx_t* ec_scheduler_gates,
                                coordinator* coordinator_actor){
  ec_scheduler_batch_ = ec_scheduler_batch;
  ec_scheduler_gates_ = ec_scheduler_gates;
  coordinator_actor_ = coordinator_actor;
}
