#include "coordinator.h"
#include "flow_actor_allocator.h"
#include "../actor/base/local_send.h"
#include "../nf/base/network_function_register.h"

#include <glog/logging.h>

inline void process_recv_reliable_msg(reliable_single_msg* msg_ptr){
  LOG(INFO)<<"received the msg from rtm: "<<msg_ptr->send_runtime_id;

}

coordinator::coordinator(flow_actor_allocator* allocator,
                         generic_ring_allocator<generic_list_item>* mac_list_item_allocator,
                         llring_holder& holder){
  allocator_ = allocator;

  htable_.Init(flow_key_size, sizeof(flow_actor*));

  deadend_flow_actor_ = allocator_->allocate();

  nfa_ipv4_field::nfa_init_ipv4_field(fields_);

  static_nf_register::get_register().init(allocator->get_max_actor());
  service_chain_ = static_nf_register::get_register().get_service_chain(0x0000000000000001);

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
}

void coordinator::handle_message(es_scheduler_pkt_batch_t, bess::PacketBatch* batch){
  ec_scheduler_batch_.clear();
  char keys[bess::PacketBatch::kMaxBurst][flow_key_size] __ymm_aligned;

  for(int i=0; i<batch->cnt(); i++){
    char* data_start = reinterpret_cast<char *>(batch->pkts()[i]->buffer());
    data_start += batch->pkts()[i]->data_off();

    if(unlikely( ((*((uint16_t*)(data_start+14)) & 0x00f0) != 0x0040) ||
                 ( ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x0006) &&
                   ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x0011) &&
                   ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x00FF)) ) ){
      gp_collector_.collect(batch->pkts()[i]);
      continue;
    }

    if(unlikely(((*((uint16_t*)(data_start+23)) & 0x00ff) == 0x00FF))){
      uint64_t mac_addr = ((*(reinterpret_cast<uint64_t *>(data_start))) & 0x0000FFffFFffFFfflu);

      auto it = mac_to_reliables_.find(mac_addr);
      if(unlikely(it == mac_to_reliables_.end())){
        gp_collector_.collect(batch->pkts()[i]);
        continue;
      }

      reliable_single_msg* msg_ptr = it->second.recv(batch->pkts()[i]);

      if(unlikely(msg_ptr != nullptr)){
        process_recv_reliable_msg(msg_ptr);
        msg_ptr->clean(&gp_collector_);
      }

      continue;
    }

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
        send(actor, flow_actor_init_t::value,
             this, reinterpret_cast<flow_key_t*>(keys[i]), service_chain_);
      }

      htable_.Set(reinterpret_cast<flow_key_t*>(keys[i]), &actor);

      actor_ptr = &actor;
    }

    send(*actor_ptr, pkt_msg_t::value, batch->pkts()[i]);
  }
}

void coordinator::handle_message(remove_flow_t, flow_actor* flow_actor, flow_key_t* flow_key){

  htable_.Del(flow_key);

  if(flow_actor!=deadend_flow_actor_){
    allocator_->deallocate(flow_actor);
  }
  else{
  }
}

void coordinator::handle_message(control_pkts_batch_t, bess::PacketBatch* batch){
  ec_scheduler_batch_.clear();

  for(int i=0; i<batch->cnt(); i++){
    char* data_start = reinterpret_cast<char *>(batch->pkts()[i]->buffer());
    data_start += batch->pkts()[i]->data_off();

    if(unlikely( ((*((uint16_t*)(data_start+14)) & 0x00f0) != 0x0040) ||
                 (((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x00FF)) ) ){
      gp_collector_.collect(batch->pkts()[i]);
      continue;
    }

    uint64_t mac_addr = ((*(reinterpret_cast<uint64_t *>(data_start))) & 0x0000FFffFFffFFfflu);

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
