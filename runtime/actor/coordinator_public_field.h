#ifndef COORDINATOR_PUBLIC_FIELD
#define COORDINATOR_PUBLIC_FIELD

#include <list>
#include <unordered_map>

#include "../utils/round_rubin_list.h"
#include "../utils/fast_hash_map.h"
#include "../bessport/module.h"
#include "../bessport/kmod/llring.h"
#include "../rpc/ring_msg.h"
#include "../actor/flow_actor.h"
#include "../reliable/reliable_p2p.h"
#include "./base/garbage_pkt_collector.h"
#include "../utils/generic_list_item.h"
#include "actor_timer_list.h"

struct garbage{
  garbage_pkt_collector gp_collector_;
};

struct local_batch{
  bess::PacketBatch ec_scheduler_batch_;
  uint16_t ec_scheduler_gates_[bess::PacketBatch::kMaxBurst];
};

struct timer_list{
  actor_timer_list<actor_timer_type::flow_actor_idle_timer> idle_flow_list_;
  actor_timer_list<actor_timer_type::flow_actor_req_timer> req_timer_list_;
};

struct rpcworker_llring{
  struct llring* rpc2worker_ring_;
  struct llring* worker2rpc_ring_;
};

struct local_runtime_info{
  runtime_config local_runtime_;
  uint64_t default_input_mac_;
  uint64_t default_output_mac_;
};

struct rr_lists{
  generic_ring_allocator<generic_list_item>* mac_list_item_allocator_;

  round_rubin_list<generic_list_item> output_runtime_mac_rrlist_;
  round_rubin_list<generic_list_item> input_runtime_mac_rrlist_;

  round_rubin_list<flow_actor> active_flows_rrlist_;

  round_rubin_list<generic_list_item> replicas_rrlist_;

  round_rubin_list<generic_list_item> reliable_send_list_;
};

struct migration_target_source_holder{
  uint64_t migration_qouta_;

  int32_t migration_target_rt_id_;
};

struct reliables_holder{
  fast_hash_map<uint32_t, reliable_p2p, uint32_keycmp, uint32_hash> reliables_;
  HTable<uint64_t, reliable_p2p*, uint64_keycmp, uint64_hash> mac_to_reliables_;
};

#endif
