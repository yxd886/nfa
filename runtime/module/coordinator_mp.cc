#include "coordinator_mp.h"
#include "../actor/coordinator_messages.h"

#include <glog/logging.h>


void coordinator_mp::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
  send_flag = false;
}

struct task_result coordinator_mp::RunTask(void *arg){
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  auto it = coordinator_actor_->reliables_.find(3);

  if(it!=coordinator_actor_->reliables_.end() && send_flag==false){
    ping_cstruct cstruct;
    it->second.reliable_send(2, 1, 1, ping_t::value, &cstruct);
    send_flag = true;
  }

  return ret;
}

ADD_MODULE(coordinator_mp, "coordinator_mp", "send messages to another mp")
