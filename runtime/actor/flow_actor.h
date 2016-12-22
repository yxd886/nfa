#ifndef FLOW_ACTOR_H
#define FLOW_ACTOR_H

#include <iostream>
#include <list>
#include <vector>

#include "../bessport/packet.h"
#include "../bessport/pktbatch.h"
#include "../nf/base/nf_item.h"
#include "./base/actor.h"
#include "./base/flow_key.h"
#include "flow_actor_messages.h"
#include "fixed_timer.h"

using namespace std;

class coordinator;
class flow_actor : public actor_base{
public:
  void handle_message(flow_actor_init_t,
                      coordinator* coordinator_actor,
                      flow_key_t* flow_key,
                      nf_item* nf_items,
                      size_t service_chain_length);

  void handle_message(pkt_msg_t, bess::Packet* pkt);

  void handle_message(check_idle_t);

  flow_actor() : pkt_counter_(0), sample_counter_(0), idle_counter_(0), coordinator_actor_(0),
      service_chain_length_(0){}

  inline flow_key_t* peek_flow_key(){
    return &flow_key_;
  }

  inline nf_item* peek_nf_items(){
    return nf_items_;
  }

  inline size_t peek_service_chain_length(){
    return service_chain_length_;
  }

private:
  uint64_t pkt_counter_;
  uint64_t sample_counter_;
  int idle_counter_;

  flow_key_t flow_key_;
  coordinator* coordinator_actor_;

  nf_item nf_items_[8];
  size_t service_chain_length_;
};

#endif
