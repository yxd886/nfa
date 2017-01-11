#ifndef COORDINATOR_MESSAGES_H
#define COORDINATOR_MESSAGES_H

#include "./base/local_message.h"
#include "./base/flow_key.h"
#include "./base/flow_ether_header.h"

enum class coordinator_messages : uint16_t{
  remove_flow,
  ping,

  create_migration_target_actor,

  change_vswitch_route
};

using remove_flow_t = local_message(coordinator_messages, remove_flow);

using ping_t = local_message(coordinator_messages, ping);
struct ping_cstruct{
  int val;
};

using create_migration_target_actor_t = local_message(coordinator_messages, create_migration_target_actor);
struct create_migration_target_actor_cstruct{
  flow_ether_header input_header;
  flow_ether_header output_header;
  flow_key_t flow_key;
};

using change_vswitch_route_t = local_message(coordinator_messages, change_vswitch_route);
struct change_vswitch_route_request_cstruct{
  flow_key_t flow_key;
  uint32_t new_output_rt_id;
  uint64_t new_output_rt_input_mac;
};

#endif
