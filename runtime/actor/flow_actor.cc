#include "flow_actor.h"
#include "coordinator.h"
#include "./base/local_send.h"
#include "../bessport/utils/time.h"
#include "deduplication.h"

void flow_actor::handle_message(flow_actor_init_with_pkt_t,
                                coordinator* coordinator_actor,
                                flow_key_t* flow_key,
                                vector<network_function_base*>& service_chain,
                                bess::Packet* first_packet){
  flow_key_ = *flow_key;
  coordinator_actor_ = coordinator_actor;

  pkt_counter_ = 0;
  sample_counter_ = 0;

  int32_t input_rtid;
  uint64_t input_rt_output_mac =  (*(first_packet->head_data<uint64_t*>(6)) & 0x0000FFffFFffFFfflu);
  reliable_p2p** r_ptr = coordinator_actor->mac_to_reliables_.Get(&input_rt_output_mac);
  if(unlikely(r_ptr == nullptr)){
    input_rtid = 0;
  }
  else{
    input_rtid = (*r_ptr)->get_rt_config()->runtime_id;
  }
  input_header_.init(input_rtid, input_rt_output_mac, coordinator_actor->local_runtime_.input_port_mac);

  int32_t output_rtid;
  uint64_t output_rt_input_mac;
  generic_list_item* first_item = coordinator_actor->output_runtime_mac_rrlist_.rotate();
  if(unlikely(first_item==nullptr)){
    output_rtid = 0;
    output_rt_input_mac = coordinator_actor->default_output_mac_;
  }
  else{
    output_rt_input_mac = first_item->dst_mac_addr;
    output_rtid = first_item->dst_rtid;
  }
  output_header_.init(output_rtid, output_rt_input_mac, coordinator_actor->local_runtime_.output_port_mac);

  size_t i = 0;
  service_chain_length_ = service_chain.size();

  for(; i<service_chain_length_; i++){
    char* fs_state_ptr = service_chain[i]->allocate();

    if(unlikely(fs_state_ptr == nullptr)){
      LOG(WARNING)<<"flow state allocation failed";
      for(size_t j=0; j<i; j++){
        nfs_.nf[j]->deallocate(fs_.nf_flow_state_ptr[j]);
      }
      service_chain_length_ = 0;
      break;
    }

    nfs_.nf[i] = service_chain[i];
    fs_.nf_flow_state_ptr[i] = fs_state_ptr;
    fs_size_.nf_flow_state_size[i] = service_chain[i]->get_nf_state_size();
  }

  coordinator_actor_->idle_flow_list_.add_timer(&idle_timer_,
                                                ctx.current_ns(),
                                                idle_message_id,
                                                static_cast<uint16_t>(flow_actor_messages::check_idle));


  if(FLAGS_deduplicate_flag&&is_duplicate_packet(first_packet)){

  	coordinator_actor_->migration_target_rt_id_=FLAGS_deduplicate_rtm_id;
  	handle_message(start_migration_t::value, FLAGS_deduplicate_rtm_id);

  }
}

void flow_actor::handle_message(flow_actor_init_with_cstruct_t,
                                coordinator* coordinator_actor,
                                flow_key_t* flow_key,
                                vector<network_function_base*>& service_chain,
                                create_migration_target_actor_cstruct* cstruct){
  flow_key_ = *flow_key;
  coordinator_actor_ = coordinator_actor;

  pkt_counter_ = 0;
  sample_counter_ = 0;

  input_header_.init(cstruct->input_header.dest_rtid,
                     &(cstruct->input_header.ethh.d_addr),
                     coordinator_actor->local_runtime_.input_port_mac);
  output_header_.init(cstruct->output_header.dest_rtid,
                      &(cstruct->output_header.ethh.d_addr),
                      coordinator_actor->local_runtime_.output_port_mac);

  size_t i = 0;
  service_chain_length_ = service_chain.size();

  for(; i<service_chain_length_; i++){
    char* fs_state_ptr = service_chain[i]->allocate();

    if(unlikely(fs_state_ptr == nullptr)){
      LOG(WARNING)<<"flow state allocation failed";
      for(size_t j=0; j<i; j++){
        nfs_.nf[j]->deallocate(fs_.nf_flow_state_ptr[j]);
      }
      service_chain_length_ = 0;
      break;
    }

    nfs_.nf[i] = service_chain[i];
    fs_.nf_flow_state_ptr[i] = fs_state_ptr;
    fs_size_.nf_flow_state_size[i] = service_chain[i]->get_nf_state_size();
  }

  coordinator_actor_->idle_flow_list_.add_timer(&idle_timer_,
                                                ctx.current_ns(),
                                                idle_message_id,
                                                static_cast<uint16_t>(flow_actor_messages::check_idle));
}

void flow_actor::handle_message(pkt_msg_t, bess::Packet* pkt){
  pkt_counter_+=1;

  // output phase, ogate 0 of ec_scheduler is connected to the output port.
  // ogate 1 of ec_scheduler is connected to a sink


  if(FLAGS_deduplicate_target_flag&&is_duplicate_packet(pkt)){

  	coordinator_actor_->gp_collector_.collect(pkt);

  }else{

  	for(size_t i=0; i<service_chain_length_; i++){
  		rte_prefetch0(fs_.nf_flow_state_ptr[i]);
  		nfs_.nf[i]->nf_logic(pkt, fs_.nf_flow_state_ptr[i]);
  	}

  	rte_memcpy(pkt->head_data(), &(output_header_.ethh), sizeof(struct ether_hdr));

  	coordinator_actor_->ec_scheduler_batch_.add(pkt);

  }

}

void flow_actor::handle_message(check_idle_t){
  idle_timer_.invalidate();

  if(sample_counter_ == pkt_counter_){
    for(size_t i=0; i<service_chain_length_; i++){
      nfs_.nf[i]->deallocate(fs_.nf_flow_state_ptr[i]);
    }

    send(coordinator_actor_, remove_flow_t::value, this, &flow_key_);
  }
  else{
    sample_counter_ = pkt_counter_;
    coordinator_actor_->idle_flow_list_.add_timer(&idle_timer_,
                                                  ctx.current_ns(),
                                                  idle_message_id,
                                                  static_cast<uint16_t>(flow_actor_messages::check_idle));
  }
}

void flow_actor::handle_message(start_migration_t, int32_t migration_target_rtid){
  create_migration_target_actor_cstruct cstruct;
  rte_memcpy(&(cstruct.input_header), &input_header_, sizeof(flow_ether_header));
  rte_memcpy(&(cstruct.output_header), &output_header_, sizeof(flow_ether_header));
  rte_memcpy(&(cstruct.flow_key), &flow_key_, sizeof(flow_key_t));

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  bool flag = coordinator_actor_->reliables_.find(migration_target_rtid)->reliable_send(
                                      msg_id,
                                      actor_id_,
                                      coordinator_actor_id,
                                      create_migration_target_actor_t::value,
                                      &cstruct);

  if(flag == false){
    // do some processing
    return;
  }

  coordinator_actor_->req_timer_list_.add_timer(&migration_timer_,
                                                ctx.current_ns(),
                                                msg_id,
                                                static_cast<uint16_t>(flow_actor_messages::start_migration_timeout));
}

void flow_actor::handle_message(start_migration_timeout_t){
  migration_timer_.invalidate();
  LOG(INFO)<<"start_migration_timeout is triggered";
}

void flow_actor::handle_message(start_migration_response_t, start_migration_response_cstruct* cstruct_ptr){
  if(unlikely(cstruct_ptr->request_msg_id != migration_timer_.request_msg_id_)){
    LOG(INFO)<<"The timer has been triggered, the response is autoamtically discared";
    return;
  }

  LOG(INFO)<<"The response is successfully received, the id of the migration target is "
           <<cstruct_ptr->migration_target_actor_id;
  migration_timer_.invalidate();

  migration_target_actor_id_ = cstruct_ptr->migration_target_actor_id;

  change_vswitch_route_request_cstruct cstruct;
  cstruct.new_output_rt_id = coordinator_actor_->migration_target_rt_id_;
  cstruct.new_output_rt_input_mac = cstruct_ptr->migration_target_input_mac;
  rte_memcpy(&(cstruct.flow_key), &flow_key_, sizeof(flow_key_t));

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  bool flag = coordinator_actor_->reliables_.find(input_header_.dest_rtid)->reliable_send(
                                        msg_id,
                                        actor_id_,
                                        coordinator_actor_id,
                                        change_vswitch_route_t::value,
                                        &cstruct);

  if(flag == false){
    // do some processing
    return;
  }

  coordinator_actor_->req_timer_list_.add_timer(&migration_timer_,
                                                ctx.current_ns(),
                                                msg_id,
                                                static_cast<uint16_t>(flow_actor_messages::change_vswitch_route_timeout));
}

void flow_actor::handle_message(change_vswitch_route_timeout_t){
  migration_timer_.invalidate();
  LOG(INFO)<<"change_vswitch_route_timeout is triggered";
}

void flow_actor::handle_message(change_vswtich_route_execution_t,
                                int32_t new_output_rtid,
                                uint64_t new_output_rt_input_mac){
  output_header_.dest_rtid = new_output_rtid;
  output_header_.ethh.d_addr = *(reinterpret_cast<struct ether_addr*>(&new_output_rt_input_mac));
}

void flow_actor::handle_message(change_vswitch_route_response_t, change_vswitch_route_response_cstruct* cstruct_ptr){
  if(unlikely(cstruct_ptr->request_msg_id != migration_timer_.request_msg_id_)){
    LOG(INFO)<<"The timer has been triggered, the response is autoamtically discared";
    return;
  }

  LOG(INFO)<<"The response is successfully received, the route has been changed";
  migration_timer_.invalidate();

  bess::Packet* pkt = bess::Packet::Alloc();
  pkt->set_data_off(SNBUF_HEADROOM);
  pkt->set_total_len(fs_size_.nf_flow_state_size[0]);
  pkt->set_data_len(fs_size_.nf_flow_state_size[0]);
  bess::PacketBatch batch;
  batch.clear();
  batch.add(pkt);

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  bool flag = coordinator_actor_->reliables_.find(coordinator_actor_->migration_target_rt_id_)->reliable_send(
                                                  msg_id,
                                                  actor_id_,
                                                  migration_target_actor_id_,
                                                  migrate_flow_state_t::value,
                                                  &batch);

  if(flag == false){
    coordinator_actor_->gp_collector_.collect(&batch);
    return;
  }

  coordinator_actor_->req_timer_list_.add_timer(&migration_timer_,
                                                ctx.current_ns(),
                                                msg_id,
                                                static_cast<uint16_t>(flow_actor_messages::migrate_flow_state_timeout));
}

void flow_actor::handle_message(migrate_flow_state_t,
                                int32_t sender_rtid,
                                uint32_t sender_actor_id,
                                uint32_t request_msg_id,
                                bess::PacketBatch* fs_pkt_batch){
  LOG(INFO)<<"Receive fs_pkt_batch!!!";

  uint32_t msg_id = coordinator_actor_->allocate_msg_id();
  migrate_flow_state_response_cstruct cstruct;
  cstruct.request_msg_id = request_msg_id;

  bool flag = coordinator_actor_->reliables_.find(sender_rtid)->reliable_send(
                                                  msg_id,
                                                  actor_id_,
                                                  sender_actor_id,
                                                  migrate_flow_state_response_t::value,
                                                  &cstruct);

  if(flag == false){
    return;
  }

}

void flow_actor::handle_message(migrate_flow_state_timeout_t){
  migration_timer_.invalidate();
  LOG(INFO)<<"Receive migrate_flow_state_timeout";
}

void flow_actor::handle_message(migrate_flow_state_response_t, migrate_flow_state_response_cstruct* cstruct_ptr){
  if(unlikely(cstruct_ptr->request_msg_id != migration_timer_.request_msg_id_)){
    LOG(INFO)<<"The timer has been triggered, the response is autoamtically discared";
    return;
  }

  LOG(INFO)<<"The response is successfully received, the migration has completed";
  migration_timer_.invalidate();
}
