#ifndef FIXED_TIMER_H
#define FIXED_TIMER_H

#include <cstdint>

#include "fixed_timer_messages.h"

static constexpr uint64_t flow_actor_idle_timeout = 1*1000*1000*1000;

template<uint64_t TO>
struct fixed_timer{
  uint64_t timer_to_trigger;
  void* actor_ptr;
  fixed_timer_messages msg;

  void trigger();

  fixed_timer(uint64_t now, void* actor_p, fixed_timer_messages m) :
    timer_to_trigger(now+TO), actor_ptr(actor_p), msg(m){}
};

#endif
