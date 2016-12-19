#ifndef COORDINATOR_H
#define COORDINATOR_H

#include "../bessport/utils/htable.h"
#include "../bessport/pktbatch.h"
#include "../bessport/worker.h"
#include "./base/actor.h"
#include "./base/nfa_ipv4_field.h"
#include "./base/flow_hash.h"
#include "coordinator_messages.h"

class flow_actor;
class flow_actor_allocator;

class coordinator : public actor_base{
public:
  using htable_t = HTable<flow_key_t, flow_actor*, flow_keycmp, flow_hash>;

  coordinator(flow_actor_allocator* allocator);

  void handle_message(es_scheduler_pkt_batch_t, bess::PacketBatch* batch);

  inline bess::PacketBatch* peek_ec_scheduler_batch(){
    return &ec_scheduler_batch_;
  }

  inline gate_idx_t* peek_ec_scheduler_gates(){
   return es_scheduler_gates_;
  }

private:
  flow_actor_allocator* allocator_;
  htable_t htable_;
  flow_actor* deadend_flow_actor_;
  nfa_ipv4_field fields_[3];

  bess::PacketBatch ec_scheduler_batch_;
  gate_idx_t es_scheduler_gates_[bess::PacketBatch::kMaxBurst];
};

#endif
