#ifndef FLOW_ACTOR_MESSAGES_H
#define FLOW_ACTOR_MESSAGES_H

#include "./base/local_message.h"

enum class flow_actor_messages : uint16_t{
  pkt_msg,
  flow_actor_init_with_pkt,
  flow_actor_init_with_cstruct,
  check_idle,

  start_migration,
  start_migration_response,
  start_migration_timeout,

  change_vswtich_route_execution,
  change_vswitch_route_timeout,
  change_vswitch_route_response,

  migrate_flow_state,
  migrate_flow_state_timeout,
  migrate_flow_state_response
};

using pkt_msg_t = local_message(flow_actor_messages, pkt_msg);
using flow_actor_init_with_pkt_t = local_message(flow_actor_messages, flow_actor_init_with_pkt);
using flow_actor_init_with_cstruct_t = local_message(flow_actor_messages, flow_actor_init_with_cstruct);
using check_idle_t = local_message(flow_actor_messages, check_idle);

using start_migration_t = local_message(flow_actor_messages, start_migration);
using start_migration_timeout_t = local_message(flow_actor_messages, start_migration_timeout);
using start_migration_response_t = local_message(flow_actor_messages, start_migration_response);
struct start_migration_response_cstruct{
  uint32_t request_msg_id;
  uint32_t migration_target_actor_id;
  uint64_t migration_target_input_mac;
};

using change_vswtich_route_execution_t = local_message(flow_actor_messages, change_vswtich_route_execution);
using change_vswitch_route_timeout_t = local_message(flow_actor_messages, change_vswitch_route_timeout);
using change_vswitch_route_response_t = local_message(flow_actor_messages, change_vswitch_route_response);
struct change_vswitch_route_response_cstruct{
  uint32_t request_msg_id;
};


using migrate_flow_state_t = local_message(flow_actor_messages, migrate_flow_state);
using migrate_flow_state_timeout_t = local_message(flow_actor_messages, migrate_flow_state_timeout);
using migrate_flow_state_response_t = local_message(flow_actor_messages, migrate_flow_state_response);
struct migrate_flow_state_response_cstruct{
  uint32_t request_msg_id;
};




#endif
