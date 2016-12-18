#ifndef FLOW_ACTOR_MESSAGES_H
#define FLOW_ACTOR_MESSAGES_H

#include "local_message.h"

enum class flow_actor_messages{
  pkt_msg
};

using pkt_msg_t = local_message(flow_actor_messages, pkt_msg);

#endif
