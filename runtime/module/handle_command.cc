//
#include "handle_command.h"
#include "../bessport/kmod/llring.h"

#include <glog/logging.h>

void handle_command::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
}

void handle_command::add_input_output_runtime(llring_item* item){
  // coordinator_actor_->rtid_to_input_output_rt_config_.emplace(item->rt_config.runtime_id,
  //                                                                     item->rt_config);
  LOG(INFO)<<"Adding input port mac "<<convert_uint64t_mac(item->rt_config.input_port_mac)
           <<" to mac_addr_to_rt_configs_";
  coordinator_actor_->mac_addr_to_rt_configs_.emplace(item->rt_config.input_port_mac,
                                                      item->rt_config);

  LOG(INFO)<<"Adding output port mac "<<convert_uint64t_mac(item->rt_config.output_port_mac)
           <<" to mac_addr_to_rt_configs_";
  coordinator_actor_->mac_addr_to_rt_configs_.emplace(item->rt_config.output_port_mac,
                                                      item->rt_config);
}

void handle_command::delete_input_output_runtime(llring_item* item){
  // coordinator_actor_->rtid_to_input_output_rt_config_.erase(item->rt_config.runtime_id);
  LOG(INFO)<<"Deleteing input port mac "<<convert_uint64t_mac(item->rt_config.input_port_mac)
           <<" from mac_addr_to_rt_configs_";
  coordinator_actor_->mac_addr_to_rt_configs_.erase(item->rt_config.input_port_mac);

  LOG(INFO)<<"Deleteing output port mac "<<convert_uint64t_mac(item->rt_config.output_port_mac)
           <<" from mac_addr_to_rt_configs_";
  coordinator_actor_->mac_addr_to_rt_configs_.erase(item->rt_config.output_port_mac);
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
        LOG(INFO)<<"Adding output port mac "<<convert_uint64t_mac(item->rt_config.output_port_mac)
                   <<" to mac_addr_to_rt_configs_";
        coordinator_actor_->mac_addr_to_rt_configs_.emplace(item->rt_config.output_port_mac,
                                                            item->rt_config);

        LOG(INFO)<<"constructing reliables "
                 <<convert_uint64t_mac(coordinator_actor_->local_runtime_.input_port_mac)
                 <<" -> "
                 <<convert_uint64t_mac(item->rt_config.output_port_mac);
        coordinator_actor_->reliables_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item->rt_config.runtime_id),
            std::forward_as_tuple(coordinator_actor_->local_runtime_.input_port_mac,
                                  item->rt_config.output_port_mac));
        break;
      }
      case rpc_operation::add_output_runtime :{
        LOG(INFO)<<"Adding input port mac "<<convert_uint64t_mac(item->rt_config.input_port_mac)
                 <<" to mac_addr_to_rt_configs_";
        coordinator_actor_->mac_addr_to_rt_configs_.emplace(item->rt_config.input_port_mac,
                                                            item->rt_config);

        LOG(INFO)<<"constructing reliables "
                 <<convert_uint64t_mac(coordinator_actor_->local_runtime_.output_port_mac)
                 <<" -> "
                 <<convert_uint64t_mac(item->rt_config.input_port_mac);
        coordinator_actor_->reliables_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item->rt_config.runtime_id),
            std::forward_as_tuple(coordinator_actor_->local_runtime_.output_port_mac,
                                  item->rt_config.input_port_mac));
        break;
      }
      case rpc_operation::delete_input_runtime :{
        LOG(INFO)<<"Deleteing output port mac "<<convert_uint64t_mac(item->rt_config.output_port_mac)
                 <<" from mac_addr_to_rt_configs_";
        coordinator_actor_->mac_addr_to_rt_configs_.erase(item->rt_config.output_port_mac);

        LOG(INFO)<<"Deleting reliables to "<<item->rt_config.runtime_id;
        coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);

        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->input_runtime_mac_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->dst_mac_addr == item->rt_config.output_port_mac){
            LOG(INFO)<<"Deleting "<<convert_uint64t_mac(g_item->dst_mac_addr)
                     <<" from the input_runtime_mac_rrlist_";
            coordinator_actor_->input_runtime_mac_rrlist_.list_item_delete(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }
        break;
      }
      case rpc_operation::delete_output_runtime :{
        LOG(INFO)<<"Deleteing input port mac "<<convert_uint64t_mac(item->rt_config.input_port_mac)
                 <<" from mac_addr_to_rt_configs_";
        coordinator_actor_->mac_addr_to_rt_configs_.erase(item->rt_config.input_port_mac);

        LOG(INFO)<<"Deleting reliables to "<<item->rt_config.runtime_id;
        coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);

        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->output_runtime_mac_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->dst_mac_addr == item->rt_config.input_port_mac){
            LOG(INFO)<<"Deleting "<<convert_uint64t_mac(g_item->dst_mac_addr)
                     <<" from the output_runtime_mac_rrlist_";
            coordinator_actor_->output_runtime_mac_rrlist_.list_item_delete(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }
        break;
      }
      case rpc_operation::add_input_mac :{
        generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();
        list_item->dst_mac_addr = item->rt_config.output_port_mac;
        LOG(INFO)<<"Adding "<<convert_uint64t_mac(list_item->dst_mac_addr)<<" to input_runtime_mac_rrlist_";
        coordinator_actor_->input_runtime_mac_rrlist_.add_to_tail(list_item);
        break;
      }
      case rpc_operation::add_output_mac :{
        generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();
        list_item->dst_mac_addr = item->rt_config.input_port_mac;
        LOG(INFO)<<"Adding "<<convert_uint64t_mac(list_item->dst_mac_addr)<<" to output_runtime_mac_rrlist_";
        coordinator_actor_->output_runtime_mac_rrlist_.add_to_tail(list_item);
        break;
      }
      case rpc_operation::delete_input_mac :{
        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->input_runtime_mac_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->dst_mac_addr == item->rt_config.output_port_mac){
            LOG(INFO)<<"Deleting "<<convert_uint64t_mac(g_item->dst_mac_addr)
                                <<" from the input_runtime_mac_rrlist_";
            coordinator_actor_->input_runtime_mac_rrlist_.list_item_delete(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }
        break;
      }
      case rpc_operation::delete_output_mac :{
        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->output_runtime_mac_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->dst_mac_addr == item->rt_config.input_port_mac){
            LOG(INFO)<<"Deleting "<<convert_uint64t_mac(g_item->dst_mac_addr)
                                 <<" from the output_runtime_mac_rrlist_";
            coordinator_actor_->output_runtime_mac_rrlist_.list_item_delete(c_item);
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }
        break;
      }
      case rpc_operation::can_migrate :{
        LOG(INFO)<<"returning migration_qouta "<<coordinator_actor_->migration_qouta_;
        item->migration_qouta = coordinator_actor_->migration_qouta_;
        break;
      }
      case rpc_operation::set_migration_target :{
        coordinator_actor_->migration_qouta_ = item->migration_qouta;
        if(coordinator_actor_->migration_target_rt_id_ == item->rt_config.runtime_id){
          LOG(INFO)<<"The new migration target is the same as the old one, reset the reliable";
          coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)->second.reset();
        }
        else{

          LOG(INFO)<<"erasing old reliables to migration target "<<coordinator_actor_->migration_target_rt_id_;
          coordinator_actor_->reliables_.erase(coordinator_actor_->migration_target_rt_id_);

          LOG(INFO)<<"adding new migration target reliable "
                   <<convert_uint64t_mac(coordinator_actor_->local_runtime_.control_port_mac)
                   <<" -> "
                   <<convert_uint64t_mac(item->rt_config.control_port_mac);
          coordinator_actor_->reliables_.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(item->rt_config.runtime_id),
                      std::forward_as_tuple(coordinator_actor_->local_runtime_.control_port_mac,
                                            item->rt_config.control_port_mac));
        }
        break;
      }
      case rpc_operation::migration_negotiate :{
        //first of all, determine the number of migration that can be accepted. Using the number of
        // available flow actors.
        // TODO:

        if(coordinator_actor_->reliables_.find(item->rt_config.runtime_id) !=
            coordinator_actor_->reliables_.end()){
          LOG(INFO)<<"The new migration source is the same as the old one, reset the reliable";
          coordinator_actor_->reliables_.find(item->rt_config.runtime_id)->second.reset();
        }
        else{
          LOG(INFO)<<"adding new migration source reliable "
                   <<convert_uint64t_mac(coordinator_actor_->local_runtime_.control_port_mac)
                   <<" -> "
                   <<convert_uint64t_mac(item->rt_config.control_port_mac);
          coordinator_actor_->reliables_.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(item->rt_config.runtime_id),
                      std::forward_as_tuple(coordinator_actor_->local_runtime_.control_port_mac,
                                            item->rt_config.control_port_mac));

          LOG(INFO)<<"adding rtid_to_migrate_in_rrlist_ for migration source "<<item->rt_config.runtime_id;
          coordinator_actor_->rtid_to_migrate_in_rrlist_.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(item->rt_config.runtime_id),
                      std::forward_as_tuple());
        }
        break;
      }
      case rpc_operation::delete_migration_target :{
        if(coordinator_actor_->migration_qouta_==0){
          coordinator_actor_->reliables_.erase(coordinator_actor_->migration_target_rt_id_);
          coordinator_actor_->migration_target_rt_id_ = -1;
          item->rt_config.runtime_id = -1;
        }
        break;
      }
      case rpc_operation::delete_migration_source :{
        // TODO: remove all the flow actors in rtid_to_migrate_in_rrlist_

        coordinator_actor_->rtid_to_migrate_in_rrlist_.erase(item->rt_config.runtime_id);
        coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
        break;
      }
      case rpc_operation::add_replica :{
        generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();
        list_item->replica_rtid_ = item->rt_config.runtime_id;
        LOG(INFO)<<"add replica "<<list_item->replica_rtid_<<" to replicas_rrlist_";
        coordinator_actor_->replicas_rrlist_.add_to_tail(list_item);

        if(coordinator_actor_->reliables_.find(item->rt_config.runtime_id) ==
            coordinator_actor_->reliables_.end()){
          LOG(INFO)<<"add reliable"
                   <<convert_uint64t_mac(coordinator_actor_->local_runtime_.control_port_mac)
                   <<" -> "
                   <<convert_uint64t_mac(item->rt_config.control_port_mac);
          coordinator_actor_->reliables_.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(item->rt_config.runtime_id),
                      std::forward_as_tuple(coordinator_actor_->local_runtime_.control_port_mac,
                                            item->rt_config.control_port_mac));
          LOG(INFO)<<"increment the reference counter for reliable";
          coordinator_actor_->reliables_.find(item->rt_config.runtime_id)->second.inc_ref_cnt();
        }
        else{
          LOG(INFO)<<"increment the reference counter for reliable";
          coordinator_actor_->reliables_.find(item->rt_config.runtime_id)->second.inc_ref_cnt();
        }
        break;
      }
      case rpc_operation::add_storage :{
        LOG(INFO)<<"creating a storage for runtime "<<item->rt_config.runtime_id;
        coordinator_actor_->storage_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item->rt_config.runtime_id),
            std::forward_as_tuple());

        if(coordinator_actor_->reliables_.find(item->rt_config.runtime_id) ==
            coordinator_actor_->reliables_.end()){
          LOG(INFO)<<"add reliable"
                   <<convert_uint64t_mac(coordinator_actor_->local_runtime_.control_port_mac)
                   <<" -> "
                   <<convert_uint64t_mac(item->rt_config.control_port_mac);
          coordinator_actor_->reliables_.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(item->rt_config.runtime_id),
                      std::forward_as_tuple(coordinator_actor_->local_runtime_.control_port_mac,
                                            item->rt_config.control_port_mac));

          LOG(INFO)<<"increment the reference counter for reliable";
          coordinator_actor_->reliables_.find(item->rt_config.runtime_id)->second.inc_ref_cnt();
        }
        else{
          LOG(INFO)<<"increment the reference counter for reliable";
          coordinator_actor_->reliables_.find(item->rt_config.runtime_id)->second.inc_ref_cnt();
        }

        break;
      }
      case rpc_operation::remove_replica :{
        cdlist_item *c_item = nullptr;
        cdlist_for_each(c_item, coordinator_actor_->replicas_rrlist_.get_list_head()){
          generic_list_item* g_item = reinterpret_cast<generic_list_item*>(c_item);
          if(g_item->replica_rtid_ == item->rt_config.runtime_id){
            coordinator_actor_->replicas_rrlist_.list_item_delete(c_item);
            LOG(INFO)<<"removing replica "<<g_item->replica_rtid_<<" from replicas_rrlist_";
            coordinator_actor_->get_list_item_allocator()->deallocate(g_item);
            break;
          }
        }

        LOG(INFO)<<"decrement reference counter for reliable to "<<item->rt_config.runtime_id;
        reliable_p2p& r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id)->second;
        r.dec_ref_cnt();
        if(r.is_ref_cnt_zero()){
          LOG(INFO)<<"reference counter of reliable "<<item->rt_config.runtime_id<<" reaches 0, erasing";
          coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
        }

        break;
      }
      case rpc_operation::remove_storage :{
        // TODO:  remove all the storage flow actors

        LOG(INFO)<<"removing storage flow list for runtime "<<item->rt_config.runtime_id;
        coordinator_actor_->storage_.erase(item->rt_config.runtime_id);

        LOG(INFO)<<"decrement reference counter for reliable to "<<item->rt_config.runtime_id;
        reliable_p2p& r = coordinator_actor_->reliables_.find(item->rt_config.runtime_id)->second;
        r.dec_ref_cnt();
        if(r.is_ref_cnt_zero()){
          LOG(INFO)<<"reference counter of reliable "<<item->rt_config.runtime_id<<" reaches 0, erasing";
          coordinator_actor_->reliables_.erase(item->rt_config.runtime_id);
        }

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
