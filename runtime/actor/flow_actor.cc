#include "flow_actor.h"
#include "coordinator.h"
#include "./base/local_send.h"
#include "../bessport/utils/time.h"

void flow_actor::handle_message(flow_actor_init_t,
                                coordinator* coordinator_actor,
                                flow_key_t* flow_key,
                                vector<network_function_base*>& service_chain){
  flow_key_ = *flow_key;
  coordinator_actor_ = coordinator_actor;

  pkt_counter_ = 0;
  sample_counter_ = 0;
  idle_counter_ = 0;

  service_chain_length_ = service_chain.size();
  for(size_t i=0; i<service_chain_length_; i++){
    nf_items_[i].nf = service_chain[i];
    nf_items_[i].nf_flow_state_ptr = service_chain[i]->allocate();
    nf_items_[i].nf_flow_state_size = service_chain[i]->get_nf_state_size();
  }

  add_timer(coordinator_actor_->peek_idle_flow_check_list(),
            ctx.current_ns(), static_cast<void*>(this), fixed_timer_messages::empty_msg);
}

void flow_actor::handle_message(pkt_msg_t, bess::Packet* pkt){
  pkt_counter_+=1;

  // output phase, ogate 0 of ec_scheduler is connected to the output port.
  // ogate 1 of ec_scheduler is connected to a sink
  for(size_t i=0; i<service_chain_length_; i++){
    nf_items_[i].nf->nf_logic(pkt, nf_items_[i].nf_flow_state_ptr);
  }

  int next_available_pos = coordinator_actor_->peek_ec_scheduler_batch()->cnt();
  coordinator_actor_->peek_ec_scheduler_gates()[next_available_pos] = 0;
  coordinator_actor_->peek_ec_scheduler_batch()->add(pkt);
}

void flow_actor::handle_message(check_idle_t){
  if(sample_counter_ == pkt_counter_){
    idle_counter_ += 1;
    if(idle_counter_ == 3){
      for(size_t i=0; i<service_chain_length_; i++){
        nf_items_[i].nf->deallocate(nf_items_[i].nf_flow_state_ptr);
      }
      send(coordinator_actor_, remove_flow_t::value, this, &flow_key_);
    }
    else{
      add_timer(coordinator_actor_->peek_idle_flow_check_list(),
                    ctx.current_ns(), static_cast<void*>(this), fixed_timer_messages::empty_msg);
    }
  }
  else{
    idle_counter_ = 0;
    sample_counter_ = pkt_counter_;
    add_timer(coordinator_actor_->peek_idle_flow_check_list(),
              ctx.current_ns(), static_cast<void*>(this), fixed_timer_messages::empty_msg);
  }
}