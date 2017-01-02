#ifndef COORDINATOR_MESSAGES_H
#define COORDINATOR_MESSAGES_H

#include "./base/local_message.h"

static constexpr uint16_t coordinator_messages_start = 0x0001;

enum class coordinator_messages : uint16_t{
  es_scheduler_pkt_batch = coordinator_messages_start,
  remove_flow,
  recv_reliable_msg,
  control_pkts_batch
};

using es_scheduler_pkt_batch_t = local_message(coordinator_messages, es_scheduler_pkt_batch);
using remove_flow_t = local_message(coordinator_messages, remove_flow);
using recv_reliable_msg_t = local_message(coordinator_messages, recv_reliable_msg);
using control_pkts_batch_t = local_message(coordinator_messages, control_pkts_batch);

#endif
