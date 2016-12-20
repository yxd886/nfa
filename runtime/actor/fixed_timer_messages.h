#ifndef FIXED_TIMER_MESSAGES_H
#define FIXED_TIMER_MESSAGES_H

#include <cstdint>

enum class fixed_timer_messages{
  empty_msg
};

static constexpr uint64_t flow_actor_idle_timeout = 1*1000*1000*1000;

#endif

