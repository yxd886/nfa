#ifndef FLOW_ACTOR_H
#define FLOW_ACTOR_H

#include <iostream>
#include <list>
#include <vector>

#include "../bessport/packet.h"
#include "../bessport/pktbatch.h"
#include "./base/flow_key.h"
#include "./base/flow_ether_header.h"
#include "flow_actor_messages.h"
#include "../nf/base/network_function_base.h"
#include "../nf/base/nf_item.h"
#include "../utils/cdlist.h"
#include "actor_timer.h"

using namespace std;

class coordinator;
class flow_actor{
public:
  using flow_actor_id_t = uint32_t;

  void handle_message(flow_actor_init_t,
                      coordinator* coordinator_actor,
                      flow_key_t* flow_key,
                      vector<network_function_base*>& service_chain,
                      uint32_t input_rtid,
                      uint64_t input_rt_output_mac,
                      uint64_t local_rt_input_mac,
                      uint32_t output_rtid,
                      uint64_t output_rt_input_mac,
                      uint64_t local_rt_output_mac);

  void handle_message(pkt_msg_t, bess::Packet* pkt);

  void handle_message(check_idle_t);

  inline flow_actor_id_t* get_id(){
    return &actor_id_;
  }

  inline void set_id(flow_actor_id_t actor_id){
    actor_id_ = actor_id;
  }

  inline actor_timer<actor_timer_type::flow_actor_idle_timer>* get_idle_timer(){
    return &idle_timer_;
  }

private:
  struct cdlist_item list_item;

  flow_actor_id_t actor_id_;

  uint64_t pkt_counter_;

  uint64_t sample_counter_;

  uint64_t idle_counter_;

  flow_key_t flow_key_;

  coordinator* coordinator_actor_;

  flow_ether_header input_header_;

  flow_ether_header output_header_;

  size_t service_chain_length_;

  flow_actor_nfs nfs_;

  flow_actor_fs fs_;

  flow_actor_fs_size fs_size_;

  actor_timer<actor_timer_type::flow_actor_idle_timer> idle_timer_;
};

static_assert(std::is_pod<flow_actor>::value, "flow_actor is not pod");

#endif
