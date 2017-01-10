#include "coordinator_mp.h"
#include "../actor/coordinator_messages.h"

#include <glog/logging.h>


void coordinator_mp::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
  num_to_send = 100000000;
  successful_send = 0;
  unsuccessful_send= 0;
  send_end_flag = false;
}

struct task_result coordinator_mp::RunTask(void *arg){
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  /*if(coordinator_actor_->migration_target_rt_id_ != -1){
    ping_cstruct cstruct;
    cstruct.val = 1024;

    for(int i=0; i<32; i++){
      bool flag = coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)->second
                                    .reliable_send(77363, 1, 1, ping_t::value, &cstruct);
      if(flag==false){
        unsuccessful_send+=1;
      }
      else{
        successful_send+=1;
      }
    }

    if(successful_send%30000000 == 0){
      LOG(INFO)<<"Unsuccessful send "<<unsuccessful_send;
      LOG(INFO)<<"Successful send "<<successful_send;
      LOG(INFO)<<"The rtt is "
               <<coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)->second.peek_rtt()
               <<"ns";
    }
  }*/

  if(unlikely(coordinator_actor_->migration_qouta_>0)){
    for(int i=0; i<32; i++){
      flow_actor* actor_ptr = coordinator_actor_->active_flows_rrlist_.pop_head();
      if(actor_ptr==nullptr){
        break;
      }

      coordinator_actor_->migration_qouta_ -= 1;
      send(actor_ptr, start_migration_t::value, coordinator_actor_->migration_target_rt_id_);
    }
  }

  return ret;
}

ADD_MODULE(coordinator_mp, "coordinator_mp", "send messages to another mp")
