#ifndef COORDINATOR_MESSAGES_H
#define COORDINATOR_MESSAGES_H

#include "./base/local_message.h"
#include "./base/flow_key.h"

enum class coordinator_messages : uint16_t{
  dp_pkt_batch,
  cp_pkt_batch,
  remove_flow,
  ping,

  create_migration_target_actor
};

using dp_pkt_batch_t = local_message(coordinator_messages, dp_pkt_batch);
using cp_pkt_batch_t = local_message(coordinator_messages, cp_pkt_batch);
using remove_flow_t = local_message(coordinator_messages, remove_flow);

using ping_t = local_message(coordinator_messages, ping);
struct ping_cstruct{
  int val;
};

using create_migration_target_actor_t = local_message(coordinator_messages, create_migration_target_actor);
struct create_migration_target_actor_cstruct{
  int32_t input_rtid;
  int32_t output_rtid;
  flow_key_t flow_key;
};

#endif
