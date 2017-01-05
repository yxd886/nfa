#ifndef COORDINATOR_MESSAGES_H
#define COORDINATOR_MESSAGES_H

#include "./base/local_message.h"

enum class coordinator_messages : uint16_t{
  dp_pkt_batch,
  cp_pkt_batch,
  remove_flow,
  ping
};

using dp_pkt_batch_t = local_message(coordinator_messages, dp_pkt_batch);
using cp_pkt_batch_t = local_message(coordinator_messages, cp_pkt_batch);
using remove_flow_t = local_message(coordinator_messages, remove_flow);

using ping_t = local_message(coordinator_messages, ping);
struct ping_cstruct{
  int val;
};

#endif
