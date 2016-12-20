#ifndef FLOW_ACTOR_H
#define FLOW_ACTOR_H

#include <iostream>
#include <list>

#include "../bessport/packet.h"
#include "../bessport/pktbatch.h"
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
                      flow_key_t* flow_key);

  void handle_message(pkt_msg_t, bess::Packet* pkt);

  void handle_message(check_idle_t);

  flow_actor() : pkt_counter_(0), sample_counter_(0), idle_counter_(0), coordinator_actor_(0){}

private:
  uint64_t pkt_counter_;
  uint64_t sample_counter_;
  int idle_counter_;

  flow_key_t flow_key_;
  coordinator* coordinator_actor_;
};

#endif
