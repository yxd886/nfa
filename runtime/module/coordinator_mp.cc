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

  if( coordinator_actor_->local_runtime_.runtime_id==2 &&
      coordinator_actor_->reliables_.find(3)!=nullptr &&
      send_end_flag == false){
    ping_cstruct cstruct;
    cstruct.val = 1024;
    bool flag = coordinator_actor_->reliables_.find(3)
                                  ->reliable_send(77363, 1, 1, ping_t::value, &cstruct);
    if(flag==false){
      LOG(INFO)<<"Fail to send the message";
    }
    else{
      LOG(INFO)<<"Succeed to send the message";
    }

    send_end_flag = true;
  }

  if( coordinator_actor_->local_runtime_.runtime_id==3 &&
      coordinator_actor_->reliables_.find(2)!=nullptr &&
      send_end_flag == false){
    ping_cstruct cstruct;
    cstruct.val = 1024;
    bool flag = coordinator_actor_->reliables_.find(2)
                                  ->reliable_send(77363, 1, 1, ping_t::value, &cstruct);
    if(flag==false){
      LOG(INFO)<<"Fail to send the message";
    }
    else{
      LOG(INFO)<<"Succeed to send the message";
    }

    send_end_flag = true;
  }

  /*if(coordinator_actor_->migration_target_rt_id_!=-1 && send_end_flag == false){
    ping_cstruct cstruct;
    cstruct.val = 1024;
    bool flag = coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)
                                  ->reliable_send(77363, 1, 1, ping_t::value, &cstruct);
    if(flag==false){
      LOG(INFO)<<"Fail to send the message";
    }
    else{
      LOG(INFO)<<"Succeed to send the message";
    }

    send_end_flag = true;
  }*/

  /*if(coordinator_actor_->migration_target_rt_id_ != -1 && send_end_flag == false){
    ping_cstruct cstruct;
    cstruct.val = 1024;

    for(int i=0; i<32; i++){
      bool flag = coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)
                                    ->reliable_send(77363, 1, 1, ping_t::value, &cstruct);
      if(flag==false){
        unsuccessful_send+=1;
      }
      else{
        successful_send+=1;
      }
    }

    if(successful_send == 32*2500000){
      LOG(INFO)<<"Unsuccessful send "<<unsuccessful_send;
      LOG(INFO)<<"Successful send "<<successful_send;
      LOG(INFO)<<"The rtt is "
               <<coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)->peek_rtt()
               <<"ns";
      send_end_flag = true;
    }
  }*/

  /*for(int i=0; i<32; i++){
    if((coordinator_actor_->migration_qouta_==0) || (coordinator_actor_->outgoing_migrations_>1024)){
      break;
    }

    flow_actor* actor_ptr = coordinator_actor_->active_flows_rrlist_.peek_head();
    if(actor_ptr==nullptr){
      coordinator_actor_->migration_qouta_ -= 1;

      coordinator_actor_->null_passive_migration_ += 1;
      continue;
    }

    coordinator_actor_->active_flows_rrlist_.pop_head();
    coordinator_actor_->migration_qouta_ -= 1;
    coordinator_actor_->outgoing_migrations_ += 1;
    send(actor_ptr, start_migration_t::value, coordinator_actor_->migration_target_rt_id_);
  }


  cdlist_head* replica_flow_list = coordinator_actor_->replica_flow_lists_.find(coordinator_actor_->storage_rtid_);
  for(int i=0; i<32; i++){
    if(coordinator_actor_->storage_rtid_ == -1 || coordinator_actor_->out_going_recovery_>1024){
      break;
    }

    cdlist_item* replica_flow = cdlist_pop_head(replica_flow_list);
    if(unlikely(replica_flow == nullptr)){
      if(coordinator_actor_->out_going_recovery_ == 0){
        coordinator_actor_->storage_rtid_ = -1;
        coordinator_actor_->current_recovery_iteration_end_time_ = ctx.current_ns();
      }
      break;
    }

    reinterpret_cast<flow_actor*>(replica_flow)->start_recover();

    coordinator_actor_->out_going_recovery_ += 1;
  }


  if(current_iteration<coordinator_actor_->passive_migration_iteration_){
    if(coordinator_actor_->successful_passive_migration_ +
       coordinator_actor_->failed_passive_migration_ +
       coordinator_actor_->null_passive_migration_  == coordinator_actor_->total_passive_migration_){
      LOG(INFO)<<"The migration qouta : "<<coordinator_actor_->total_passive_migration_<<" flows";
      LOG(INFO)<<"Successful migration : "<<coordinator_actor_->successful_passive_migration_;
      LOG(INFO)<<"Failed migration : "<<coordinator_actor_->failed_passive_migration_;
      LOG(INFO)<<"Null migration : "<<coordinator_actor_->null_passive_migration_;
      LOG(INFO)<<"migration_source_loss_counter : "<<coordinator_actor_->migration_source_loss_counter_;
      uint64_t time = ctx.current_ns() -  coordinator_actor_->current_iteration_start_time_;
      time = time/1000000;
      LOG(INFO)<<"Migration takes "<<time<<"ms.";

      current_iteration+=1;
    }
  }

  if(coordinator_actor_->migrated_in_flow_num_ == 50000 && send_end_flag==false){
    LOG(INFO)<<"The migration_target_loss_counter is "
             <<coordinator_actor_->migration_target_loss_counter_;
    LOG(INFO)<<"The migration_target_buffer_size_counter is "
             <<coordinator_actor_->migration_target_buffer_size_counter_;
    LOG(INFO)<<"The average migration_target_buffer_size is "
             <<coordinator_actor_->migration_target_buffer_size_counter_/coordinator_actor_->migrated_in_flow_num_;
    send_end_flag = true;
  }

  if( (local_replication_iteration < coordinator_actor_->recovery_iteration_) &&
      (coordinator_actor_->storage_rtid_ == -1) ){
    LOG(INFO)<<"Successful recovery : "<<coordinator_actor_->successful_recovery_;
    LOG(INFO)<<"Failed recovery : "<<coordinator_actor_->unsuccessful_recovery_;

    uint64_t time = coordinator_actor_->current_recovery_iteration_end_time_ -
        coordinator_actor_->current_recovery_iteration_start_time_;
    time = time/1000000;
    LOG(INFO)<<"Recovery takes "<<time<<"ms.";

    local_replication_iteration += 1;
  }*/

  return ret;
}

ADD_MODULE(coordinator_mp, "coordinator_mp", "send messages to another mp")
