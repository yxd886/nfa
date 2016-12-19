#ifndef FLOW_ACTOR_MESSAGES_H
#define FLOW_ACTOR_MESSAGES_H

#include "./base/local_message.h"

enum class flow_actor_messages{
  pkt_msg,
  ec_scheduler_batch_and_gates
};

using pkt_msg_t = local_message(flow_actor_messages, pkt_msg);
using ec_scheduler_batch_and_gates_t = local_message(flow_actor_messages, ec_scheduler_batch_and_gates);

#endif
