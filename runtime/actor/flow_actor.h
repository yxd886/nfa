#ifndef FLOW_ACTOR_H
#define FLOW_ACTOR_H

#include <iostream>

#include "../bessport/packet.h"
#include "../bessport/pktbatch.h"

#include "./base/local_send.h"
#include "./base/flow_key.h"
#include "flow_actor_messages.h"

using namespace std;

class flow_actor : public actor_base{
public:
  void handle_message(pkt_msg_t, bess::Packet* pkt);
  void handle_message(ec_scheduler_batch_and_gates_t, bess::PacketBatch* ec_scheduler_batch, gate_idx_t* ec_scheduler_gates);

  flow_actor() : pkt_counter(0), ec_scheduler_batch_(0), ec_scheduler_gates_(0){}

private:
  uint64_t pkt_counter;

  bess::PacketBatch* ec_scheduler_batch_;
  gate_idx_t* ec_scheduler_gates_;
};

#endif
