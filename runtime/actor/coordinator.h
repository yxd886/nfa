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
#include "../utils/generic_ring_allocator.h"
#include "coordinator_public_field.h"

class flow_actor;
class flow_actor_allocator;

class coordinator : public garbage, public local_batch, public timer_list,
                    public rpcworker_llring, public local_runtime_info,
                    public active_flows, public input_output_runtime_info,
                    public migration_target_source_holder, public replicas_holder,
                    public storages_holder, public reliables_holder,
                    public reliable_send_record{
public:
  using htable_t = HTable<flow_key_t, flow_actor*, flow_keycmp, flow_hash>;

  coordinator(flow_actor_allocator* allocator,
              generic_ring_allocator<generic_list_item>* mac_list_item_allocator,
              llring_holder& holder);

  void process_recv_reliable_msg(reliable_single_msg* msg_ptr);

  void handle_message(dp_pkt_batch_t, bess::PacketBatch* batch);

  void handle_message(cp_pkt_batch_t, bess::PacketBatch* batch);

  void handle_message(remove_flow_t, flow_actor* flow_actor, flow_key_t* flow_key);

  void handle_message(ping_t, int32_t sender_rtid, uint32_t sender_actor_id, uint32_t msg_id,
                      ping_cstruct* cstruct_ptr);

  inline generic_ring_allocator<generic_list_item>* get_list_item_allocator(){
    return mac_list_item_allocator_;
  }

private:

  flow_actor_allocator* allocator_;

  htable_t htable_;

  flow_actor* deadend_flow_actor_;

  nfa_ipv4_field fields_[3];

  std::vector<network_function_base*> service_chain_;

  generic_ring_allocator<generic_list_item>* mac_list_item_allocator_;

  int counter = 0;
  uint64_t start_time = 0;
};

#endif
