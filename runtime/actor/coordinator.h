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

class coordinator : public local_batch, public timer_list, public rpcworker_llring,
                    public input_output_runtime_info, public migration_target_source_holder,
                    public replicas_holder, public storages_holder, public reliables_holder{
public:
  using htable_t = HTable<flow_key_t, flow_actor*, flow_keycmp, flow_hash>;

  coordinator(flow_actor_allocator* allocator,
              generic_ring_allocator<generic_list_item>* mac_list_item_allocator,
              llring_holder& holder);

  void handle_message(es_scheduler_pkt_batch_t, bess::PacketBatch* batch);

  void handle_message(remove_flow_t, flow_actor* flow_actor, flow_key_t* flow_key);

private:
  flow_actor_allocator* allocator_;

  htable_t htable_;

  flow_actor* deadend_flow_actor_;

  nfa_ipv4_field fields_[3];

  std::vector<network_function_base*> service_chain_;

  runtime_config local_runtime_;

  generic_ring_allocator<generic_list_item>* mac_list_item_allocator_;

};

#endif
