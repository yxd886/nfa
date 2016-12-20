#include "fixed_timer.h"
#include "flow_actor.h"
#include "./base/local_send.h"

template<>
void fixed_timer<flow_actor_idle_timeout>::trigger(){
  send(static_cast<flow_actor*>(actor_ptr), check_idle_t::value);
}
