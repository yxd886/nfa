#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <list>
#include <unordered_map>

#include "../bessport/utils/htable.h"
#include "../bessport/pktbatch.h"
#include "../bessport/worker.h"
#include "./base/nfa_ipv4_field.h"
#include "./base/flow_hash.h"
#include "coordinator_messages.h"
#include "fixed_timer.h"
#include "../nf/base/network_function_base.h"
#include "../rpc/ring_msg.h"
#include "../nfaflags.h"
#include "../rpc/llring_holder.h"

class flow_actor;
class flow_actor_allocator;

class coordinator{
public:
  using htable_t = HTable<flow_key_t, flow_actor*, flow_keycmp, flow_hash>;

  coordinator(flow_actor_allocator* allocator, llring_holder& holder);

  void handle_message(es_scheduler_pkt_batch_t, bess::PacketBatch* batch);

  void handle_message(remove_flow_t, flow_actor* flow_actor, flow_key_t* flow_key);

  inline bess::PacketBatch* peek_ec_scheduler_batch(){
    return &ec_scheduler_batch_;
  }

  inline gate_idx_t* peek_ec_scheduler_gates(){
   return es_scheduler_gates_;
  }

  inline std::list<fixed_timer<flow_actor_idle_timeout>>* peek_idle_flow_check_list(){
    return &idle_flow_check_list_;
  }

  inline struct llring* peek_rpc2worker_ring(){
    return rpc2worker_ring_;
  }

  inline struct llring* peek_worker2rpc_ring(){
    return worker2rpc_ring_;
  }

private:
  flow_actor_allocator* allocator_;

  htable_t htable_;

  flow_actor* deadend_flow_actor_;

  nfa_ipv4_field fields_[3];

  bess::PacketBatch ec_scheduler_batch_;

  gate_idx_t es_scheduler_gates_[bess::PacketBatch::kMaxBurst];

  struct llring* rpc2worker_ring_;

  struct llring* worker2rpc_ring_;

  std::list<fixed_timer<flow_actor_idle_timeout>> idle_flow_check_list_;

  std::vector<network_function_base*> service_chain_;

  std::unordered_map<int32_t, runtime_config> input_runtimes_;

  std::unordered_map<int32_t, runtime_config> output_runtimes_;

  std::unordered_map<int32_t, runtime_config> replicas_;

  std::unordered_map<int32_t, runtime_config> storages_;

  runtime_config migration_target_;

  uint64_t migration_qouta_;

  runtime_config local_runtime_;
};

#endif
