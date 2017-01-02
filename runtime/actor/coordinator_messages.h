#ifndef COORDINATOR_MESSAGES_H
#define COORDINATOR_MESSAGES_H

#include "./base/local_message.h"

static constexpr uint16_t coordinator_messages_start = 0x0001;

enum class coordinator_messages : uint16_t{
  es_scheduler_pkt_batch = coordinator_messages_start,
  remove_flow
};

using es_scheduler_pkt_batch_t = local_message(coordinator_messages, es_scheduler_pkt_batch);
using remove_flow_t = local_message(coordinator_messages, remove_flow);

#endif
