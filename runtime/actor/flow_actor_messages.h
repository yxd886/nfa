#ifndef FLOW_ACTOR_MESSAGES_H
#define FLOW_ACTOR_MESSAGES_H

#include "./base/local_message.h"

static constexpr uint16_t flow_actor_messages_start = 0x8001;

enum class flow_actor_messages : uint16_t{
  pkt_msg = flow_actor_messages_start,
  flow_actor_init,
  check_idle
};

using pkt_msg_t = local_message(flow_actor_messages, pkt_msg);
using flow_actor_init_t = local_message(flow_actor_messages, flow_actor_init);
using check_idle_t = local_message(flow_actor_messages, check_idle);

#endif
