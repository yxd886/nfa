#ifndef FIXED_TIMER_H
#define FIXED_TIMER_H

#include <list>

#include "fixed_timer_messages.h"

template<uint64_t TO>
struct fixed_timer{
  uint64_t timer_to_trigger;
  void* actor_ptr;
  fixed_timer_messages msg;

  void trigger();

  fixed_timer(uint64_t now, void* actor_p, fixed_timer_messages m) :
    timer_to_trigger(now+TO), actor_ptr(actor_p), msg(m){}
};

template<uint64_t TO>
inline bool timeout_occur(std::list<fixed_timer<TO>>* to_list, uint64_t now){
  return to_list->empty() ?
          false :
          ((to_list->front().timer_to_trigger>now) ? false : true);
}

template<uint64_t TO>
inline void trigger_timer(std::list<fixed_timer<TO>>* to_list){
  to_list->front().trigger();
  to_list->pop_front();
}

template<uint64_t TO, class... Args>
inline void add_timer(std::list<fixed_timer<TO>>* to_list, Args&&... args){
  to_list->push_back(fixed_timer<TO>(std::forward<Args>(args)...));
}

#endif
