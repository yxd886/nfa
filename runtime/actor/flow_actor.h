#ifndef FLOW_ACTOR_H
#define FLOW_ACTOR_H

#include <iostream>

#include "../bessport/packet.h"

#include "local_send.h"
#include "flow_actor_messages.h"

using namespace std;

static constexpr int flow_key_field_size = 2;

static constexpr int flow_key_size = 16;

struct flow_key_t{
  uint64_t field[flow_key_field_size];
};

class flow_actor : public actor_base{
public:
  void handle_message(pkt_msg_t, bess::Packet* pkt);

  flow_actor() : pkt_counter(0){}

private:
  uint64_t pkt_counter;
};

#endif
