#ifndef ACTOR_TIMER_H
#define ACTOR_TIMER_H

#include "../utils/round_rubin_list.h"
#include "./base/actor_type.h"
#include "./base/local_send.h"
#include "./base/actor_timer_list_type.h"
#include "./base/actor_id.h"
#include "coordinator.h"
#include "flow_actor.h"

template<timer_list_type T>
struct actor_timer{
  cdlist_item list_item_;

  // ptr to the actor, may be a flow_actor pointer
  // or coordinator actor pointer
  void* actor_ptr_;

  // type of the actor_ptr, according t
  uint16_t type_;

  uint64_t to_time_;

  uint32_t request_msg_id_;

  uint16_t msg_type_;

  // called at initialization time.
  inline void init(uint16_t type, void* actor_ptr){
    type_ = type;
    actor_ptr_ = actor_ptr;
    request_msg_id_ = invalid_message_id;
    cdlist_item_init(&list_item_);
  }

  // called when the actor is deallocated,
  // or after the timer has finished triggering,
  // or after the response associated with the request has been received.
  inline void invalidate(){
    request_msg_id_ = invalid_message_id;
    cdlist_del(&list_item_);
  }

  inline bool request_timeout(uint32_t request_msg_id){
    return (request_msg_id_==request_msg_id);
  }

  inline void trigger(){
    assert(request_msg_id_ != invalid_message_id);
    switch(static_cast<actor_type>(type_)){
      case actor_type::coordinator_actor :
        send(static_cast<coordinator*>(actor_ptr_), local_message_derived<msg_type_>::value);
        break;
      case actor_type::flow_actor :
        assert(static_cast<flow_actor*>(actor_ptr_)->get_id() != invalid_flow_actor_id);
        send(static_cast<flow_actor*>(actor_ptr_), local_message_derived<msg_type_>::value);
        break;
      default:
        break;
    }
  }
};

static_assert(std::is_pod<actor_timer<timer_list_type::flow_actor_idle_timer>>::value,
    "actor_timer is not a pod");

static_assert(std::is_pod<actor_timer<timer_list_type::flow_actor_req_timer>>::value,
    "actor_timer is not a pod");
#endif
