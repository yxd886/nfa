#ifndef FLOW_ACTOR_MESSAGES_H
#define FLOW_ACTOR_MESSAGES_H

#include "./base/local_message.h"

enum class flow_actor_messages : uint16_t{
  pkt_msg,
  flow_actor_init,
  check_idle,
  start_migration,
  start_migration_response,
  start_migration_timeout
};

using pkt_msg_t = local_message(flow_actor_messages, pkt_msg);
using flow_actor_init_t = local_message(flow_actor_messages, flow_actor_init);
using check_idle_t = local_message(flow_actor_messages, check_idle);

using start_migration_t = local_message(flow_actor_messages, start_migration);
using start_migration_timeout_t = local_message(flow_actor_messages, start_migration_timeout);
using start_migration_response_t = local_message(flow_actor_messages, start_migration_response);
struct start_migration_response_cstruct{
  uint32_t request_msg_id;
};


#endif
