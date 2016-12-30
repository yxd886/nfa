//
#include "handle_command.h"
#include "../bessport/kmod/llring.h"
#include "../rpc/ring_msg.h"

void handle_command::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
}

struct task_result handle_command::RunTask(void *arg){
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  void* dequeue_output[1];

  int flag = llring_sc_dequeue(coordinator_actor_->rpc2worker_ring_, dequeue_output);

  if(unlikely(flag == 0)){
    // do the processing

    llring_item* item = static_cast<llring_item*>(dequeue_output[0]);

    LOG(INFO) << "Receive "<<opcode2string(item->op_code)<<" ring message.";
    print_config(item->rt_config);
    LOG(INFO) << "migration_qouta-> "<<item->migration_qouta;
    print_stat(item->op_code, item->stat);

    switch(item->op_code){
      case rpc_operation::add_input_runtime :{
        coordinator_actor_->rtid_to_input_output_rt_config_.emplace(item->rt_config.runtime_id,
                                                                    item->rt_config);

        coordinator_actor_->mac_addr_to_rt_configs_.emplace(item->rt_config.input_port_mac,
                                                            item->rt_config);

        coordinator_actor_->mac_addr_to_rt_configs_.emplace(item->rt_config.output_port_mac,
                                                            item->rt_config);
        break;
      }
      case rpc_operation::add_output_runtime :{
        coordinator_actor_->rtid_to_input_output_rt_config_.emplace(item->rt_config.runtime_id,
                                                                    item->rt_config);

        coordinator_actor_->mac_addr_to_rt_configs_.emplace(item->rt_config.input_port_mac,
                                                            item->rt_config);

        coordinator_actor_->mac_addr_to_rt_configs_.emplace(item->rt_config.output_port_mac,
                                                            item->rt_config);
        break;
      }
      case rpc_operation::delete_input_runtime :{
        coordinator_actor_->rtid_to_input_output_rt_config_.erase(item->rt_config.runtime_id);
        coordinator_actor_->mac_addr_to_rt_configs_.erase(item->rt_config.input_port_mac);
        coordinator_actor_->mac_addr_to_rt_configs_.erase(item->rt_config.output_port_mac);
        break;
      }
      case rpc_operation::delete_output_runtime :{
        coordinator_actor_->rtid_to_input_output_rt_config_.erase(item->rt_config.runtime_id);
        coordinator_actor_->mac_addr_to_rt_configs_.erase(item->rt_config.input_port_mac);
        coordinator_actor_->mac_addr_to_rt_configs_.erase(item->rt_config.output_port_mac);
        break;
      }
      case rpc_operation::set_migration_target :{
        coordinator_actor_->migration_target_rt_id_ = item->rt_config.runtime_id;
        coordinator_actor_->migration_qouta_ = item->migration_qouta;
        break;
      }
      case rpc_operation::migration_negotiate :{
        break;
      }
      case rpc_operation::add_replica :{

        break;
      }
      case rpc_operation::add_storage :{

        break;
      }
      case rpc_operation::remove_replica :{

        break;
      }
      case rpc_operation::remove_storage :{

        break;
      }
      case rpc_operation::get_stats :{
        break;
      }
      default :
        break;
    }

    llring_sp_enqueue(coordinator_actor_->worker2rpc_ring_, static_cast<void*>(item));
  }

  return ret;
}

ADD_MODULE(handle_command, "handle_command", "handle rpc command received from the rpc thread")
