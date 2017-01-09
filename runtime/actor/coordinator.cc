#include "coordinator.h"
#include "flow_actor_allocator.h"
#include "../actor/base/local_send.h"
#include "../nf/base/network_function_register.h"
#include "./base/actor_misc.h"

#include <glog/logging.h>

void coordinator::process_recv_reliable_msg(reliable_single_msg* msg_ptr){
  if(msg_ptr->rmh.recv_actor_id == coordinator_actor_id){
    switch(static_cast<coordinator_messages>(msg_ptr->rmh.msg_type)){
      case coordinator_messages::ping : {
        handle_message(ping_t::value,
                       msg_ptr->send_runtime_id,
                       msg_ptr->rmh.send_actor_id,
                       msg_ptr->rmh.msg_id,
                       msg_ptr->cstruct_pkt->head_data<ping_cstruct*>());
        break;
      }
      case coordinator_messages::create_migration_target_actor : {
        handle_message(create_migration_target_actor_t::value,
                        msg_ptr->send_runtime_id,
                        msg_ptr->rmh.send_actor_id,
                        msg_ptr->rmh.msg_id,
                        msg_ptr->cstruct_pkt->head_data<create_migration_target_actor_cstruct*>());
        break;
      }
      case coordinator_messages::change_vswitch_route : {
        handle_message(change_vswitch_route_t::value,
                       msg_ptr->send_runtime_id,
                       msg_ptr->rmh.send_actor_id,
                       msg_ptr->rmh.msg_id,
                       msg_ptr->cstruct_pkt->head_data<change_vswitch_route_request_cstruct*>());

        break;
      }
      default : {
        break;
      }
    }
  }
  else{
    uint64_t actor_id_64 = 0x00000000FfFfFfFf & msg_ptr->rmh.recv_actor_id;
    flow_actor** actor_ptr = actorid_htable_.Get(&actor_id_64);
    if(unlikely(actor_ptr == 0)){
      LOG(INFO)<<"The actor with id "<<msg_ptr->rmh.recv_actor_id<<" does not exist";
      return;
    }

    switch(static_cast<flow_actor_messages>(msg_ptr->rmh.msg_type)){
      case flow_actor_messages::start_migration_response : {
        send(*actor_ptr,
             start_migration_response_t::value,
             msg_ptr->cstruct_pkt->head_data<start_migration_response_cstruct*>());
        break;
      }
      default : {
        assert(1==0);
        break;
      }
    }
  }
}

coordinator::coordinator(flow_actor_allocator* allocator,
                         generic_ring_allocator<generic_list_item>* mac_list_item_allocator,
                         llring_holder& holder){
  allocator_ = allocator;

  htable_.Init(flow_key_size, sizeof(flow_actor*));

  actorid_htable_.Init(sizeof(uint64_t), sizeof(flow_actor*));

  deadend_flow_actor_ = allocator_->allocate();

  nfa_ipv4_field::nfa_init_ipv4_field(fields_);

  static_nf_register::get_register().init(allocator->get_max_actor());
  service_chain_ = static_nf_register::get_register().get_service_chain(0x0000000000000001);
  LOG(INFO)<<"service chain length is "<<service_chain_.size();

  mac_list_item_allocator_ = mac_list_item_allocator;

  local_runtime_.runtime_id = FLAGS_runtime_id;
  local_runtime_.input_port_mac = convert_string_mac(FLAGS_input_port_mac);
  local_runtime_.output_port_mac = convert_string_mac(FLAGS_output_port_mac);
  local_runtime_.control_port_mac = convert_string_mac(FLAGS_control_port_mac);
  local_runtime_.rpc_ip = convert_string_ip(FLAGS_rpc_ip);
  local_runtime_.rpc_port = FLAGS_rpc_port;

  rpc2worker_ring_ = holder.rpc2worker_ring();
  worker2rpc_ring_ = holder.worker2rpc_ring();

  migration_target_rt_id_ = -1;
  migration_qouta_ = 0;

  default_input_mac_ = convert_string_mac(FLAGS_default_input_mac);
  default_output_mac_ = convert_string_mac(FLAGS_default_output_mac);

  next_msg_id_ = message_id_start;

  idle_flow_list_.init_list(flow_actor_idle_timeout);

  req_timer_list_.init_list(request_timeout);
}

void coordinator::handle_message(dp_pkt_batch_t, bess::PacketBatch* batch){
  ec_scheduler_batch_.clear();
  char keys[bess::PacketBatch::kMaxBurst][flow_key_size] __ymm_aligned;

  for(int i=0; i<batch->cnt(); i++){
    char* data_start = batch->pkts()[i]->head_data<char*>();

    memset(&keys[i][flow_key_size-8], 0, sizeof(uint64_t));
    for(int j=0; j<3; j++){
      char* key = keys[i]+fields_[j].pos;
      *(uint64_t *)key = *(uint64_t *)(data_start + fields_[j].offset) & fields_[j].mask;
    }

    flow_actor** actor_ptr = htable_.Get(reinterpret_cast<flow_key_t*>(keys[i]));
    flow_actor* actor = 0;

    if(unlikely(actor_ptr==nullptr)){
      actor = allocator_->allocate();

      if(unlikely(actor==nullptr)){
        LOG(WARNING)<<"No available flow actors to allocate";
        actor = deadend_flow_actor_;
      }
      else{
        bess::Packet* pkt = batch->pkts()[i];

        int32_t input_rtid = 0;
        uint64_t input_rt_output_mac =  (*(pkt->head_data<uint64_t*>(6)) & 0x0000FFffFFffFFfflu);
        // uint64_t local_rt_input_mac = local_runtime_.input_port_mac;

        int32_t output_rtid = 0;
        uint64_t output_rt_input_mac = default_output_mac_;
        // local_rt_output_mac = local_runtime_.output_port_mac;

        auto it = mac_addr_to_rt_configs_.find(input_rt_output_mac);
        if(it!=mac_addr_to_rt_configs_.end()){
          input_rtid = it->second.runtime_id;
        }

        generic_list_item* first_item = output_runtime_mac_rrlist_.rotate();
        if(first_item!=nullptr){
          output_rt_input_mac = first_item->dst_mac_addr;
          output_rtid = mac_addr_to_rt_configs_.find(output_rt_input_mac)->second.runtime_id;
        }

        active_flows_rrlist_.add_to_tail(actor);

        send(actor, flow_actor_init_t::value,
             this,
             reinterpret_cast<flow_key_t*>(keys[i]),
             service_chain_,
             input_rtid,
             input_rt_output_mac,
             local_runtime_.input_port_mac,
             output_rtid,
             output_rt_input_mac,
             local_runtime_.output_port_mac);
      }

      htable_.Set(reinterpret_cast<flow_key_t*>(keys[i]), &actor);

      uint64_t actor_id_64 = actor->get_id_64();
      actorid_htable_.Set(&actor_id_64, &actor);

      actor_ptr = &actor;
    }

    send(*actor_ptr, pkt_msg_t::value, batch->pkts()[i]);
  }
}

void coordinator::handle_message(cp_pkt_batch_t, bess::PacketBatch* batch){
  ec_scheduler_batch_.clear();
  for(int i=0; i<batch->cnt(); i++){
    char* data_start = batch->pkts()[i]->head_data<char*>();
    uint64_t mac_addr = ((*(reinterpret_cast<uint64_t *>(data_start+6))) & 0x0000FFffFFffFFfflu);

    auto it = mac_to_reliables_.find(mac_addr);
    if(unlikely(it == mac_to_reliables_.end())){
      gp_collector_.collect(batch->pkts()[i]);
      continue;
    }

    reliable_single_msg* msg_ptr = it->second.recv(batch->pkts()[i]);
    if(unlikely(msg_ptr == nullptr)){
      continue;
    }

    process_recv_reliable_msg(msg_ptr);
    msg_ptr->clean(&gp_collector_);
  }
}


void coordinator::handle_message(remove_flow_t, flow_actor* flow_actor, flow_key_t* flow_key){

  htable_.Del(flow_key);

  uint64_t actor_id_64 = flow_actor->get_id_64();
  actorid_htable_.Del(&actor_id_64);

  if(flow_actor!=deadend_flow_actor_){
    flow_actor->get_idle_timer()->invalidate();
    flow_actor->get_migration_timer()->invalidate();
    flow_actor->get_replication_timer()->invalidate();
    active_flows_rrlist_.list_item_delete(reinterpret_cast<cdlist_item*>(flow_actor));
    allocator_->deallocate(flow_actor);
  }
  else{
  }
}

void coordinator::handle_message(ping_t, int32_t sender_rtid, uint32_t sender_actor_id, uint32_t msg_id,
                                 ping_cstruct* cstruct_ptr){

  counter += 1;

  if(counter%30000000 == 0){
    LOG(INFO)<<"Receive "<<counter<<" messages.";
  }
}

void coordinator::handle_message(create_migration_target_actor_t,
                                 int32_t sender_rtid,
                                 uint32_t sender_actor_id,
                                 uint32_t msg_id,
                                 create_migration_target_actor_cstruct* cstruct_ptr){
  LOG(INFO)<<"Receive create_migration_target_actor message sent from runtime "<<sender_rtid
           <<", actor id "<<sender_actor_id
           <<", msg id "<<msg_id
           <<", input runtime id "<<cstruct_ptr->input_rtid
           <<", output_runtime_id "<<cstruct_ptr->output_rtid;


  flow_actor* actor = allocator_->allocate();

  if(unlikely(actor==nullptr)){
    LOG(WARNING)<<"No available flow actors to allocate";
    //TODO: error handling
    return;
  }

  active_flows_rrlist_.add_to_tail(actor);

  send(actor, flow_actor_init_t::value,
       this,
       &(cstruct_ptr->flow_key),
       service_chain_,
       cstruct_ptr->input_rtid,
       cstruct_ptr->input_rt_output_mac,
       local_runtime_.input_port_mac,
       cstruct_ptr->output_rtid,
       cstruct_ptr->output_rt_input_mac,
       local_runtime_.output_port_mac);


  htable_.Set(&(cstruct_ptr->flow_key), &actor);

  uint64_t actor_id_64 = actor->get_id_64();
  actorid_htable_.Set(&actor_id_64, &actor);

  uint32_t response_msg_id = allocate_msg_id();

  start_migration_response_cstruct cstruct;
  cstruct.request_msg_id = msg_id;
  cstruct.migration_target_actor_id = actor->get_id();

  bool flag = reliables_.find(sender_rtid)->second.reliable_send(response_msg_id,
                                                                 coordinator_actor_id,
                                                                 sender_actor_id,
                                                                 start_migration_response_t::value,
                                                                 &cstruct);

  if(flag == false){
    LOG(INFO)<<"coordinator fails to send response_msg_id";
    return;
  }
}

void coordinator::handle_message(change_vswitch_route_t,
                                 int32_t sender_rtid,
                                 uint32_t sender_actor_id,
                                 uint32_t msg_id,
                                 change_vswitch_route_request_cstruct* cstruct_ptr){
  LOG(INFO)<<"Receive change_vswitch_route message sent from runtime "<<sender_rtid
           <<" with actor id "<<sender_actor_id;
}
