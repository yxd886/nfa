#ifndef FLOW_ACTOR_H
#define FLOW_ACTOR_H

#include <iostream>
#include <list>
#include <vector>

#include "../bessport/packet.h"
#include "../bessport/pktbatch.h"
#include "./base/flow_key.h"
#include "flow_actor_messages.h"
#include "fixed_timer.h"
#include "../nf/base/network_function_base.h"
#include "../nf/base/nf_item.h"
#include "../utils/cdlist.h"
#include "mp_tcp.h"

using namespace std;

class coordinator;
class flow_actor{
public:
  using flow_actor_id_t = uint32_t;

  void handle_message(flow_actor_init_t,
                      coordinator* coordinator_actor,
                      flow_key_t* flow_key,
                      vector<network_function_base*>& service_chain);

  void handle_message(pkt_msg_t, bess::Packet* pkt);

  void handle_message(check_idle_t);

  inline flow_actor_id_t get_id(){
    return actor_id_;
  }

  inline void set_id(flow_actor_id_t actor_id){
    actor_id_ = actor_id;
  }

private:
  struct cdlist_item list_item;

  flow_actor_id_t actor_id_;

  uint64_t pkt_counter_;

  uint64_t sample_counter_;

  uint64_t idle_counter_;

  flow_key_t flow_key_;

  coordinator* coordinator_actor_;

  size_t service_chain_length_;

  flow_actor_nfs nfs_;

  flow_actor_fs fs_;

  flow_actor_fs_size fs_size_;
};

static_assert(std::is_pod<flow_actor>::value, "flow_actor is not pod");

#endif
