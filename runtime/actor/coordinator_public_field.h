#ifndef COORDINATOR_PUBLIC_FIELD
#define COORDINATOR_PUBLIC_FIELD

#include <list>
#include <unordered_map>

#include "../utils/mac_list_item.h"
#include "../utils/round_rubin_list.h"
#include "../bessport/module.h"
#include "fixed_timer.h"
#include "../bessport/kmod/llring.h"
#include "../rpc/ring_msg.h"
#include "../actor/flow_actor.h"
#include "../reliable/reliable_p2p.h"
#include "./base/garbage_pkt_collector.h"

struct garbage{
  garbage_pkt_collector gp_collector_;
};

struct local_batch{
  bess::PacketBatch ec_scheduler_batch_;
  gate_idx_t es_scheduler_gates_[bess::PacketBatch::kMaxBurst];
};

struct timer_list{
  std::list<fixed_timer<flow_actor_idle_timeout>> idle_flow_check_list_;
};

struct rpcworker_llring{
  struct llring* rpc2worker_ring_;
  struct llring* worker2rpc_ring_;
};

struct local_runtime_info{
  runtime_config local_runtime_;
};

struct active_flows{
  round_rubin_list<flow_actor> active_flows_rrlist_;
};

struct input_output_runtime_info{
  // std::unordered_map<int32_t, runtime_config> rtid_to_input_output_rt_config_;
  std::unordered_map<uint64_t, runtime_config> mac_addr_to_rt_configs_;
  round_rubin_list<generic_list_item> output_runtime_mac_rrlist_;
  round_rubin_list<generic_list_item> input_runtime_mac_rrlist_;
};

struct migration_target_source_holder{
  uint64_t migration_qouta_;

  int32_t migration_target_rt_id_;
  round_rubin_list<flow_actor> migrate_out_rrlist_;

  std::unordered_map<int32_t, round_rubin_list<flow_actor>> rtid_to_migrate_in_rrlist_;
};

struct replicas_holder{
  round_rubin_list<generic_list_item> replicas_rrlist_;
};

struct storages_holder{
  std::unordered_map<int32_t, round_rubin_list<flow_actor>> storage_;
};

struct reliables_holder{
  std::unordered_map<int32_t, reliable_p2p> reliables_;
  std::unordered_map<uint64_t, reliable_p2p&> mac_to_reliables_;
};

#endif
