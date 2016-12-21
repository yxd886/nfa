#ifndef COORDINATOR_MESSAGES_H
#define COORDINATOR_MESSAGES_H

#include "./base/local_message.h"

enum class coordinator_messages{
  es_scheduler_pkt_batch,
  remove_flow
};

using es_scheduler_pkt_batch_t = local_message(coordinator_messages, es_scheduler_pkt_batch);
using remove_flow_t = local_message(coordinator_messages, remove_flow);

#endif