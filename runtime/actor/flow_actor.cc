#include "flow_actor.h"
#include "coordinator.h"
#include "./base/local_send.h"
#include "../bessport/utils/time.h"
#include "mp_tcp.h"

void flow_actor::handle_message(flow_actor_init_t,
                                coordinator* coordinator_actor,
                                flow_key_t* flow_key,
                                vector<network_function_base*>& service_chain){
  flow_key_ = *flow_key;
  coordinator_actor_ = coordinator_actor;

  pkt_counter_ = 0;
  sample_counter_ = 0;
  idle_counter_ = 0;

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

  add_timer(&(coordinator_actor_->idle_flow_check_list_),
            ctx.current_ns(), static_cast<void*>(this), fixed_timer_messages::empty_msg);
}

void flow_actor::handle_message(pkt_msg_t, bess::Packet* pkt){

  pkt_counter_+=1;

  // output phase, ogate 0 of ec_scheduler is connected to the output port.
  // ogate 1 of ec_scheduler is connected to a sink

  if(FLAGS_mptcp_flag){
  	int32_t migration_target_id;
  	if(is_mptcp_flow(pkt,
  									 coordinator_actor_->local_runtime_.runtime_id,
  									 coordinator_actor_->migration_target_rt_id_rrlist_.size(),
										 migration_target_id)){
  		//TODO:begin to migrate this flow to migration target.


  	}else{
      for(size_t i=0; i<service_chain_length_; i++){
        rte_prefetch0(fs_.nf_flow_state_ptr[i]);
        nfs_.nf[i]->nf_logic(pkt, fs_.nf_flow_state_ptr[i]);
      }
    	coordinator_actor_->ec_scheduler_batch_.add(pkt);

  	}

  }else{

    for(size_t i=0; i<service_chain_length_; i++){
      rte_prefetch0(fs_.nf_flow_state_ptr[i]);
      nfs_.nf[i]->nf_logic(pkt, fs_.nf_flow_state_ptr[i]);
    }
  	coordinator_actor_->ec_scheduler_batch_.add(pkt);



  }


}

void flow_actor::handle_message(check_idle_t){
  if(sample_counter_ == pkt_counter_){
    idle_counter_ += 1;
    if(idle_counter_ == 3){
      for(size_t i=0; i<service_chain_length_; i++){
        nfs_.nf[i]->deallocate(fs_.nf_flow_state_ptr[i]);
      }
      send(coordinator_actor_, remove_flow_t::value, this, &flow_key_);
    }
    else{
      add_timer(&(coordinator_actor_->idle_flow_check_list_),
                    ctx.current_ns(), static_cast<void*>(this), fixed_timer_messages::empty_msg);
    }
  }
  else{
    idle_counter_ = 0;
    sample_counter_ = pkt_counter_;
    add_timer(&(coordinator_actor_->idle_flow_check_list_),
              ctx.current_ns(), static_cast<void*>(this), fixed_timer_messages::empty_msg);
  }
}
