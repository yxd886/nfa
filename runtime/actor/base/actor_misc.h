#ifndef ACTOR_MISC_H
#define ACTOR_MISC_H

static constexpr int32_t invalid_flow_actor_id = 0;

static constexpr int32_t coordinator_actor_id = 1;

static constexpr int32_t flow_actor_id_start = 2;

static constexpr uint32_t invalid_message_id = 0;

static constexpr uint32_t idle_message_id = 1;

static constexpr uint32_t message_id_start = 2;

static constexpr uint64_t flow_actor_idle_timeout = 1*1000*1000*1000;

enum class actor_timer_type{
  flow_actor_req_timer,
  flow_actor_idle_timer
};

enum class actor_type : uint16_t{
  flow_actor,
  coordinator_actor
};

#endif
