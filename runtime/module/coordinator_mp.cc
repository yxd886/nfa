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

  if(coordinator_actor_->migration_target_rt_id_ != -1 && send_flag==false){
    LOG(INFO)<<"Send message to migration target";

    ping_cstruct cstruct;
    coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)->second
                                  .reliable_send(2, 1, 1, ping_t::value, &cstruct);
    send_flag = true;
  }

  return ret;
}

ADD_MODULE(coordinator_mp, "coordinator_mp", "send messages to another mp")
