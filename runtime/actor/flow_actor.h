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
#include "./base/actor_misc.h"

using namespace std;

class coordinator;

class flow_actor{
public:
  using flow_actor_id_t = uint32_t;

  void handle_message(flow_actor_init_with_pkt_t,
                      coordinator* coordinator_actor,
                      flow_key_t* flow_key,
                      vector<network_function_base*>& service_chain,
                      bess::Packet* first_packet);

  void handle_message(flow_actor_init_with_cstruct_t,
                      coordinator* coordinator_actor,
                      flow_key_t* flow_key,
                      vector<network_function_base*>& service_chain,
                      create_migration_target_actor_cstruct* cstruct);

  void handle_message(pkt_msg_t, bess::Packet* pkt);

  void handle_message(check_idle_t);

  void handle_message(start_migration_t, int32_t migration_target_rtid);

  void handle_message(start_migration_timeout_t);

  void handle_message(start_migration_response_t, start_migration_response_cstruct* cstruct_ptr);

  void handle_message(change_vswitch_route_timeout_t);

  void handle_message(change_vswitch_route_response_t, change_vswitch_route_response_cstruct* cstruct_ptr);

  void handle_message(migrate_flow_state_t,
                      int32_t sender_rtid,
                      uint32_t sender_actor_id,
                      uint32_t request_msg_id,
                      bess::PacketBatch* fs_pkt_batch);

  void handle_message(migrate_flow_state_timeout_t);

  void handle_message(migrate_flow_state_response_t, migrate_flow_state_response_cstruct* cstruct_ptr);

  inline flow_actor_id_t get_id(){
    return actor_id_;
  }

  inline uint64_t get_id_64(){
    uint64_t actor_id_64 = 0x00000000FfFfFfFf & actor_id_;
    return actor_id_64;
  }

  inline void set_id(flow_actor_id_t actor_id){
    actor_id_ = actor_id;
  }

  inline actor_timer<actor_timer_type::flow_actor_req_timer>* get_idle_timer(){
    return &idle_timer_;
  }

  inline actor_timer<actor_timer_type::flow_actor_req_timer>* get_migration_timer(){
    return &migration_timer_;
  }

  inline actor_timer<actor_timer_type::flow_actor_req_timer>* get_replication_timer(){
    return &replication_timer_;
  }

  inline void update_output_header(int32_t new_output_rtid,
                                   uint64_t new_output_rt_input_mac){
    output_header_.dest_rtid = new_output_rtid;
    output_header_.ethh.d_addr = *(reinterpret_cast<struct ether_addr*>(&new_output_rt_input_mac));
  }

private:
  struct cdlist_item list_item;

  flow_actor_id_t actor_id_;

  uint64_t pkt_counter_;

  uint64_t sample_counter_;

  flow_key_t flow_key_;

  coordinator* coordinator_actor_;

  flow_ether_header input_header_;

  flow_ether_header output_header_;

  size_t service_chain_length_;

  flow_actor_nfs nfs_;

  flow_actor_fs fs_;

  flow_actor_fs_size fs_size_;

  actor_timer<actor_timer_type::flow_actor_req_timer> idle_timer_;

  actor_timer<actor_timer_type::flow_actor_req_timer> migration_timer_;

  actor_timer<actor_timer_type::flow_actor_req_timer> replication_timer_;

  uint32_t migration_target_actor_id_;

  uint32_t current_state_;

  void failure_handling();
};

static_assert(std::is_pod<flow_actor>::value, "flow_actor is not pod");

#endif
