//
#include "execution_context.hpp"
#include <iostream>
using std::cout;
using std::endl;

#define SNBUF_HEADROOM 128

nf_execution_context::nf_execution_context(
                                           network_function_hub& hub,
                                           int local_rt_id,
                                           vector<char>& flow_identifier,
                                           const actor& worker_a,
                                           service_chain_t service_chain_type_sig,
                                           int replication_strategy,
                                           worker_output_packet_queue* output_queue) :
                                             event_based_actor(cfg),
                                             hub(hub),
                                             local_rt_id(std::move(local_rt_id)),
                                             flow_identifier(flow_identifier),
                                             worker_a(worker_a),
                                             service_chain_type_sig(service_chain_type_sig),
                                             replication_strategy(replication_strategy),
                                             s(starting_status::normal_start),
                                             // replication target specific variable
                                             migration_source_rt_id(-1),
                                             migration_source_a(unsafe_actor_handle_init),
                                             my_vswitch_a(unsafe_actor_handle_init),
                                             replication_target_rt_id(-1),
                                             // ignore recover variable
                                             pending_transaction(false),
                                             migration_target_rt_id(-1),
                                             internal_pkt_counter(0),
                                             retry_counter(0),
                                             buffer_size(100),
                                             cur_state(nf_ec_state::normal_run),
                                             replication_target_a(unsafe_actor_handle_init),
                                             migration_target_a(unsafe_actor_handle_init),
                                             nf_ec_timer_a(unsafe_actor_handle_init),
                                             pending_clear_up_vswitch_table(false),
                                             is_registered(false),
                                             output_queue(output_queue),
                                             is_bond_to_replica(false),
                                             is_replica_alive(false),
                                             set_up_entry(false){
	make_behavior();
}

nf_execution_context::nf_execution_context(
                                           network_function_hub& hub,
                                           int local_rt_id,
                                           vector<char>& flow_identifier,
                                           const actor& worker_a,
                                           service_chain_t service_chain_type_sig,
                                           int replication_strategy,
                                           // migration target specific variables
                                           int migration_source_rt_id,
                                           const actor& migration_source_a,
                                           const actor& my_vswitch_a,
                                           int replication_target_rt_id,
                                           worker_output_packet_queue* output_queue) :
                                             event_based_actor(cfg),
                                             hub(hub),
                                             local_rt_id(local_rt_id),
                                             flow_identifier(std::move(flow_identifier)),
                                             worker_a(worker_a),
                                             service_chain_type_sig(service_chain_type_sig),
                                             replication_strategy(replication_strategy),
                                             s(starting_status::migration_target),
                                             // replication target specific variable
                                             migration_source_rt_id(migration_source_rt_id),
                                             migration_source_a(migration_source_a),
                                             my_vswitch_a(my_vswitch_a),
                                             replication_target_rt_id(replication_target_rt_id),
                                             // ignore recover variable
                                             pending_transaction(false),
                                             migration_target_rt_id(-1),
                                             internal_pkt_counter(0),
                                             retry_counter(0),
                                             buffer_size(100),
                                             cur_state(nf_ec_state::normal_run),
                                             replication_target_a(unsafe_actor_handle_init),
                                             migration_target_a(unsafe_actor_handle_init),
                                             nf_ec_timer_a(unsafe_actor_handle_init),
                                             pending_clear_up_vswitch_table(false),
                                             is_registered(true),
                                             output_queue(output_queue),
                                             is_bond_to_replica(false),
                                             is_replica_alive(false),
                                             set_up_entry(false){
	make_behavior();

}

nf_execution_context::nf_execution_context(
                                           network_function_hub& hub,
                                           int local_rt_id,
                                           vector<char>& flow_identifier,
                                           const actor& worker_a,
                                           service_chain_t service_chain_type_sig,
                                           int replication_strategy,
                                           vector<vector<char>>& p0_input_packet_bufs,
                                           // vector<vector<char>>& p1_input_packet_bufs,
                                           vector<char>& scs_buf,
                                           worker_output_packet_queue* output_queue,
                                           int replication_target_rt_id) :
                                             event_based_actor(cfg),
                                             hub(hub),
                                             local_rt_id(local_rt_id),
                                             flow_identifier(std::move(flow_identifier)),
                                             worker_a(worker_a),
                                             service_chain_type_sig(service_chain_type_sig),
                                             replication_strategy(replication_strategy),
                                             s(starting_status::recovery),
                                             // replication target specific variable
                                             migration_source_rt_id(-1),
                                             migration_source_a(unsafe_actor_handle_init),
                                             my_vswitch_a(unsafe_actor_handle_init),
                                             replication_target_rt_id(replication_target_rt_id),
                                             // the recover variables
                                             p0_input_packet_bufs(std::move(p0_input_packet_bufs)),
                                             // p1_input_packet_bufs(std::move(p1_input_packet_bufs)),
                                             scs_buf_(std::move(scs_buf)),
                                             pending_transaction(false),
                                             migration_target_rt_id(-1),
                                             internal_pkt_counter(0),
                                             retry_counter(0),
                                             buffer_size(100),
                                             cur_state(nf_ec_state::normal_run),
                                             replication_target_a(unsafe_actor_handle_init),
                                             migration_target_a(unsafe_actor_handle_init),
                                             nf_ec_timer_a(unsafe_actor_handle_init),
                                             pending_clear_up_vswitch_table(false),
                                             is_registered(false),
                                             output_queue(output_queue),
                                             is_bond_to_replica(false),
                                             is_replica_alive(false),
                                             set_up_entry(false){
	make_behavior();

}

void nf_execution_context::make_behavior(){
  set_default_handler(print_and_drop);
  if(s==starting_status::normal_start){
    // normal start
    // generate the service chain
    for(int i=0; i<compute_service_chain_length(service_chain_type_sig); i++){
      auto nf_type_sig = compute_network_function(service_chain_type_sig, i);
      auto nf = hub.get_nf_from_type_sig(nf_type_sig);
      auto fs = nf->allocate_flow_state();
      service_chain.push_back(std::make_tuple(nf, std::move(fs)));
    }

    // launch the nf_ec_timer
    nf_ec_timer_a = this->spawn<nf_ec_timer>(actor_cast<actor>(this),
                                             worker_a,
                                             replication_target_rt_id,
                                             local_rt_id,
                                             flow_identifier,
                                             service_chain_type_sig,
                                             this->id());
    if(replication_strategy>0){
      // start the contacting replicas if enable_replication is set to true
      local_send(worker_a, request_replication_target::value, replication_target_rt_id, this->id());
    }
    print_normal("nf-ec is started normally, enter normal_run().");
    normal_run();
    return;
  }
  else if(s==starting_status::migration_target){
    // started as migration target
    nf_ec_timer_a = this->spawn<nf_ec_timer>(actor_cast<actor>(this),
                                             worker_a,
                                             replication_target_rt_id,
                                             local_rt_id,
                                             flow_identifier,
                                             service_chain_type_sig,
                                             this->id());
    if(replication_strategy>0){
      send(worker_a, request_replication_target::value, replication_target_rt_id, this->id());
    }
    print_migration_target("nf-ec is started as migration target, enter wait_flow_states()");
    wait_flow_states();
    return;
  }
  else{
    print_recover("the nf_ec is recovered");
    // started as recovery.
    service_chain_state scs;
    binary_deserializer ds{system(), scs_buf_};
    ds(scs);

    // first recover the flow state
    for(int i=0; i<compute_service_chain_length(service_chain_type_sig); i++){
      auto nf_type_sig = compute_network_function(service_chain_type_sig, i);
      auto nf = hub.get_nf_from_type_sig(nf_type_sig);

      if((scs.states.size()>i)&&(scs.states[i].size()>0)){
        // if the fs_bufs contains valid flow state
        // we deserialize the flow state and
        // add to the service chain.
        service_chain.push_back(std::make_tuple(nf, nf->deserialize(system(), scs.states[i])));
      }
      else{
        // otherwise we allocate an emptry flow state
        service_chain.push_back(std::make_tuple(nf, nf->allocate_flow_state()));
      }
    }
    scs_buf_.clear();

    // then handle the p0_input_packet_bufs.
    for(size_t i=0; i<p0_input_packet_bufs.size(); i++){
      struct rte_mbuf pkt_buf;
      pkt_buf.buf_addr = reinterpret_cast<void*>(p0_input_packet_bufs[i].data());
      pkt_buf.data_off = 0;
      pkt_buf.pkt_len = p0_input_packet_bufs[i].size();
      pkt_buf.data_len = p0_input_packet_bufs[i].size();

      servce_chain_process(&pkt_buf, true);
    }

    // for(size_t i=0; i<p1_input_packet_bufs.size(); i++){
    //   struct rte_mbuf pkt_buf;
    //   pkt_buf.buf_addr = reinterpret_cast<void*>(p0_input_packet_bufs[i].data());
    //   pkt_buf.data_off = SNBUF_HEADROOM;
    //   pkt_buf.pkt_len = p0_input_packet_bufs[i].size() - SNBUF_HEADROOM;
    //   pkt_buf.data_len = p0_input_packet_bufs[i].size() - SNBUF_HEADROOM;
    //
    //   auto pass = servce_chain_process(&pkt_buf, true);
    // }

    // we have successfully recovered the state of the flow
    p0_input_packet_bufs.clear();
    // p1_input_packet_bufs.clear();


    nf_ec_timer_a = this->spawn<nf_ec_timer>(actor_cast<actor>(this),
                                             worker_a,
                                             replication_target_rt_id,
                                             local_rt_id,
                                             flow_identifier,
                                             service_chain_type_sig,
                                             this->id());
    if(replication_strategy>0){
    	request_replication_target*request_replication_target_value;
      local_send(worker_a, request_replication_target_value, replication_target_rt_id, this->id());
    }
    normal_run();
    return;




  }
}

bool nf_execution_context::servce_chain_process(struct rte_mbuf* input_pkt, bool from_p0){
  if(from_p0){
    internal_pkt_counter+=1;
    // do a forward service chain process
    bool pass = true;
    for(size_t i=0; i<service_chain.size(); i++){
      auto fs = get<1>(service_chain[i]).get();
      pass = get<0>(service_chain[i])->process_pkt(input_pkt, fs);

      // if(pass == false){
      //   return pass;
      // }
    }
    return pass;
  }
  else{
    bool pass = true;
    for(int i=service_chain.size()-1; i>=0; i--){
      auto fs = get<1>(service_chain[i]).get();
      pass = get<0>(service_chain[i])->process_pkt(input_pkt, fs);

      // if(pass == false){
      //   return pass;
      // }
    }
    return pass;
  }
}

// this function is questionable....
vector<char> nf_execution_context::pkt_to_char_buf(struct rte_mbuf* pkt){
  vector<char> ret;
  ret.resize(rte_pktmbuf_data_len(pkt));
  rte_memcpy(ret.data(), rte_pktmbuf_mtod(pkt, char *), rte_pktmbuf_data_len(pkt));
  return ret;
}

void nf_execution_context::process_pkt(struct rte_mbuf* input_pkt, bool from_p0){
  if((replication_strategy==0)||(is_replica_alive == false)){
    // process
    servce_chain_process(input_pkt, from_p0);

    // send to the output queue
    while(!output_queue->try_enqueue(input_pkt)){
    }

  }
  else{
    if(set_up_entry == false){
      if(internal_pkt_counter % 3 ==0){
      	remote_send(replication_target_a, create_new_replica::value, this->id(), flow_identifier, service_chain_type_sig, 10);
      }
      servce_chain_process(input_pkt, from_p0);
      while(!output_queue->try_enqueue(input_pkt)){
      }
    }
    else{
      //replicate
      if(internal_pkt_counter % 10 ==0){
        servce_chain_process(input_pkt, from_p0);

        service_chain_state scs;
        scs.states.resize(service_chain.size());

        for(size_t i=0; i<service_chain.size(); i++){
          auto nf_ptr = get<0>(service_chain[i]);
          auto fs_ptr = get<1>(service_chain[i]).get();
          nf_ptr->serialize(system(), scs.states[i], fs_ptr);
        }

        vector<char> scs_buf;
        binary_serializer bs{system(), scs_buf};
        bs(scs);

        this->remote_send(replication_target_a, from_p0, this->id(), true, scs_buf);
      }
      else{
        // vector<char> input_pkt_buf = pkt_to_char_buf(input_pkt);
        servce_chain_process(input_pkt, from_p0);
        // this->send(replication_target_a, from_p0, this->id(), false, input_pkt_buf);
      }

      // send to the output queue
      while(!output_queue->try_enqueue(input_pkt)){
      }
    }
  }
}

void nf_execution_context::flow_finish_quit(migration_status status){
  if(status == migration_status::in_migration_source){
    send(worker_a, remove_from_migration_source_nf_ecs::value, this->id(), this->migration_target_rt_id, false);
  }
  else if(status == migration_status::in_normal_processing){
    local_send(worker_a, remove_from_active_nf_ecs::value, this->id());
  }
  else{
  }

  if(is_replica_alive){
    if(set_up_entry){
    	remote_send(replication_target_a, force_quit_atom::value, this->id());
    }
    destroy(replication_target_a);
  }

  if(replication_strategy>0){
    local_send(worker_a, remove_from_replication_helpers::value, replication_target_rt_id, this->id());
  }

  remote_send(nf_ec_timer_a, nf_ec_timer_quit::value);
  destroy(worker_a);
  destroy(nf_ec_timer_a);
  destroy(replication_target_a);

  service_chain.clear();
  quit();
}

/********************************* the following functions are all behaviors ********************/
void nf_execution_context::normal_run(){

  cur_state = nf_ec_state::normal_run;
  if(is_registered == false){
    is_registered=true;
    // register itself with the worker, so that this
    // actor could be migrated.
    local_send(worker_a, add_to_active_nf_ecs::value, this->id());
  }

}

void nf_execution_context::acquire_migration_target_actor(const actor& new_migration_target_rt_a, const actor& vswitch_a){
  cur_state = nf_ec_state::acquire_migration_target_actor;

  // ask for the migration target runtime to create a migration target actor
  pending_transaction=true;
  remote_send(new_migration_target_rt_a, std::chrono::milliseconds(5*migration_timeout_ms), //50ms deadline
                create_migration_target_actor::value, flow_identifier, service_chain_type_sig, local_rt_id,
                replication_target_rt_id);

}

void nf_execution_context::change_forwarding_path(){
  cur_state = nf_ec_state::change_forwarding_path;
  local_send(this, try_change_forwarding_path::value);
  retry_counter = 0;

}


void nf_execution_context::migrate_flow_state(){
  cur_state = nf_ec_state::migrate_flow_state;
  pending_transaction=true;

  service_chain_state scs;
  scs.states.resize(service_chain.size());
  for(size_t i=0; i<service_chain.size(); i++){
    auto nf_ptr = get<0>(service_chain[i]);

    auto fs_ptr = get<1>(service_chain[i]).get();
    nf_ptr->serialize(system(), scs.states[i], fs_ptr);
  }

  // then serialize scs
  vector<char> scs_buf;
  binary_serializer bs{system(), scs_buf};
  bs(scs);

  remote_send(migration_target_a, std::chrono::seconds(migration_timeout_ms), //10s long deadline
                try_migrate_flow_state::value, scs_buf);

}

void nf_execution_context::wait_flow_states(){
  cur_state = nf_ec_state::wait_flow_states;

}


void nf_execution_context::handle_message(uint64_t pkt_ptr, bool from_p0){

	struct rte_mbuf* pkt = reinterpret_cast<struct rte_mbuf*>(pkt_ptr);
	switch (cur_state){
	case nf_ec_state::migrate_flow_state:
    while(!output_queue->try_enqueue(pkt)){}
    break;
	case nf_ec_state::wait_flow_states:
    if(from_p0){
      if(p0_buffer.size()<buffer_size){
        p0_buffer.push_back(pkt);
      }
      else{
        while(!output_queue->try_enqueue(pkt)){
        }
      }
    }
    else{
      // temporarily ignore
    }
    break;
	default:
	  process_pkt(pkt, from_p0);
	  break;

	}



}
void nf_execution_context::handle_message(atom_type(msg_type::start_migration), int new_migration_target_rt_id, const actor& new_migration_target_rt_a,
    const actor& vswitch_a){

  if((pending_transaction==true) ||
     (pending_clear_up_vswitch_table == true) ){
    // the nf ec actively reject the migration request
    // we should tells the worker to add the nf ec back to active list
    print_normal("nf-ec refuses the migration request");
    local_send(worker_a, atom_type(msg_type::remove_from_migration_source_nf_ecs)::value, this->id(), new_migration_target_rt_id, true);
  }
  else{
    // start the migration
    print_normal("nf-ec accepts the migration request, become migration source, enter acquire_migration_target_actor()");
    migration_target_rt_id = new_migration_target_rt_id;
    acquire_migration_target_actor(new_migration_target_rt_a, vswitch_a);
  }

}

void nf_execution_context::handle_message(atom_type(msg_type::set_up_entry_ok_atom)){
	set_up_entry = true;

}
void nf_execution_context::handle_message(int new_replication_target_rt_id, const actor& new_replication_target_a, bool is_replica_alive_){
  if(new_replication_target_rt_id!=-1){
    is_bond_to_replica = true;
    replication_target_rt_id = new_replication_target_rt_id;

    is_replica_alive = is_replica_alive_;
    replication_target_a = new_replication_target_a;

    if(is_replica_alive == true){
      set_up_entry = false;
    }
  }
}
void nf_execution_context::handle_message(atom_type(msg_type::rep_peer_fail)){
  is_replica_alive = false;
  destroy(replication_target_a);
}
void nf_execution_context::handle_message(atom_type(msg_type::srep_peer_back_to_alive), const actor& new_replication_target_a){
  is_replica_alive = true;
  set_up_entry = false;
  replication_target_a = new_replication_target_a;
}
void nf_execution_context::handle_message(atom_type(msg_type::idle_kill)){
  // killed due to idleness.
	switch (cur_state){
	case nf_ec_state::normal_run:
	  print_normal("nf-ec is killed due to idleness");
	  flow_finish_quit(migration_status::in_normal_processing);
	  break;
	case nf_ec_state::acquire_migration_target_actor:
		flow_finish_quit(migration_status::in_migration_source);
		break;
	case nf_ec_state::change_forwarding_path:
		destroy(my_vswitch_a);
		destroy(migration_target_a);
		flow_finish_quit(migration_status::in_migration_source);
		break;
	case nf_ec_state::wait_flow_states:
    print_migration_target("nf-ec is killed due to idleness, quit");

    // notify the worker to remove this actor from migration target list.
    local_send(worker_a, remove_from_migration_target_nf_ecs::value, this->id(), this->migration_source_rt_id, false);
    target_clean_up();
    break;
	}


}


void nf_execution_context::handle_message(atom_type(msg_type::clean_up_vswitch_table_finish)){
  pending_clear_up_vswitch_table = false;
}

void nf_execution_context::handle_message(atom_type(msg_type::nfactor_ok_atom), const actor& new_migration_target_a){

  pending_transaction = false;
  if(cur_state == nf_ec_state::acquire_migration_target_actor){
    print_migration_source("nf-ec get the migration target actor, enter change_forwarding_path()");
    migration_target_a = new_migration_target_a;
    my_vswitch_a = vswitch_a;
    change_forwarding_path();
  }
}

void nf_execution_context::handle_message(const error& err){

  switch(cur_state){
  case nf_ec_state::acquire_migration_target_actor:
  	pending_transaction = false;
  	print_migration_source("nf-ec fails to get migration target actor, become normal");
		lcoal_send(worker_a, atom_type(msg_type::remove_from_migration_source_nf_ecs)::value, this->id(), this->migration_target_rt_id, true);
		normal_run();
		break;
  case nf_ec_state::change_forwarding_path:
  	pending_transaction=false;
    retry_counter+=1;
    if(retry_counter<2){
      print_migration_source("nf-ec fails to receive error message in change_forwarding_path, retry");
      local_send(this, atom_type(msg_type::try_change_forwarding_path)::value);
    }
    else{
      // nf ec fails to change the route
      // we should tells the worker to add the nf ec back to active list
      print_migration_source("nf-ec fails to change forwarding path, become normal");
      local_send(worker_a, atom_type(msg_type::remove_from_migration_source_nf_ecs)::value, this->id(), this->migration_target_rt_id, true);
      source_clean_up();
    }
    break;
  case nf_ec_state::migrate_flow_state:
		pending_transaction=false;
		// nf ec fails to migrate the flow state
		// we should tells the worker to add the nf ec back to active list
		print_migration_source("nf-ec receives error message "+caf::to_string(err)+" in migrate_flow_state(), become normal");
		local_send(worker_a, atom_type(msg_type::remove_from_migration_source_nf_ecs)::value, this->id(), this->migration_target_rt_id, true);
		source_clean_up();
		break;

  }

}

void nf_execution_context::handle_message(atom_type(msg_type::migration_fail)){
  // the nf ec is implicitly removed from the
  // migration list and added to the active list

	switch(cur_state){
	case nf_ec_state::acquire_migration_target_actor:
		print_migration_source("nf-ec receives migraiton_fail message in acquire_migration_target_actor(), become normal");
	  normal_run();
	  break;
	case nf_ec_state::change_forwarding_path:
		print_migration_source("nf-ec receives migration_fail in change_forwarding_path, become normal");
		source_clean_up();
		break;
	case nf_ec_state::migrate_flow_state:
    print_migration_source("nf-ec receives migration_fail message in migrate_flow_state(), become normal");
    source_clean_up();
    break;
	case nf_ec_state::wait_flow_states:
    print_migration_target("nf-ec receives migration_fail message in wait_flow_state(), quit");
    target_clean_up();
    break;

	}

}

void nf_execution_context::handle_message(atom_type(msg_type::try_change_forwarding_path)){
  pending_transaction=true;
  remote_send(my_vswitch_a, std::chrono::milliseconds(5*migration_timeout_ms), // 50ms deadline
                forward_to_migration_target_actor::value,
                flow_identifier,
                migration_target_rt_id);
}

void nf_execution_context::handle_message(atom_type(msg_type::nfactor_ok_atom)){

	pending_transaction=false;
	switch(cur_state){
	case nf_ec_state::change_forwarding_path:
		print_migration_source("nf-ec successfully changes the forwarding path, enter migrate_flow_state()");
		migrate_flow_state();
		break;
	case nf_ec_state::migrate_flow_state:
		destroy(my_vswitch_a);
		destroy(migration_target_a);
		print_migration_source("nf-ec compeltes migration, quit");
		flow_finish_quit(migration_status::in_migration_source);
		break;
	}

}




void nf_execution_context::handle_message(atom_type(msg_type::try_migrate_flow_state), vector<char>& scs_buf){
  service_chain_state scs;
  binary_deserializer ds{system(), scs_buf};
  ds(scs);

  // process the flow states
  for(int i=0; i<compute_service_chain_length(service_chain_type_sig); i++){
    auto nf_type_sig = compute_network_function(service_chain_type_sig, i);
    auto nf = hub.get_nf_from_type_sig(nf_type_sig);
    auto fs = hub.get_fs_from_type_sig(nf, scs.states[i]);
    service_chain.push_back(std::make_tuple(nf, std::move(fs)));
  }

  auto rp = make_response_promise();
  rp.deliver(atom_type(msg_type::nfactor_ok_atom)::value);

  // destroy the migration_source_a and my_vswitch_a
  destroy(migration_source_a);
  destroy(my_vswitch_a);

  // notify the worker to add us to the active_nf_ec.
  local_send(worker_a, remove_from_migration_target_nf_ecs::value, this->id(), this->migration_source_rt_id, false);
  print_migration_target("nf-ec receives flow state, become normal, enter process_buffer()");

  for(size_t i=0; i<p0_buffer.size(); i++){
    process_pkt(p0_buffer[i], true);
  }

  // for(size_t i=0; i<p1_buffer.size(); i++){
  //   process_pkt(p0_buffer[i], false);
  // }

  print_migration_target("nf-ec enter normal_run()");
  normal_run();
}



void nf_execution_context::print_normal(string content){
  // aout(this)<<"INFO:[normal nf-ec "<<local_rt_id<<":"<<this->id()<<"]: "<<content<<endl;
}

void nf_execution_context::print_migration_source(string content){
  // aout(this)<<"INFO:[migration source "<<local_rt_id<<":"<<this->id()<<"]: "<<content<<endl;
}

void nf_execution_context::print_migration_target(string content){
  // aout(this)<<"INFO:[migration target "<<local_rt_id<<":"<<this->id()<<"]: "<<content<<endl;
}

void nf_execution_context::print_replication(int new_replication_target_rt_id, const actor& new_replication_target_a){
  // if(new_replication_target_rt_id==-1){
  //   aout(this)<<"INFO:[nf-ec "<<local_rt_id<<":"<<this->id()<<"]: fails to get a valid replication target runtime"<<endl;
  // }
  // else if(new_replication_target_a == actor(unsafe_actor_handle_init)){
  //   aout(this)<<"INFO:[nf-ec "<<local_rt_id<<":"<<this->id()<<"]: replication target actor fails"<<endl;
  // }
  // else{
  //   aout(this)<<"INFO:[nf-ec "<<local_rt_id<<":"<<this->id()<<"]: get a valid replication target on runtime "
  //             <<new_replication_target_rt_id<<endl;
  // }
}

void nf_execution_context::print_recover(string content){
  // aout(this)<<"INFO:[recover nf-ec "<<local_rt_id<<":"<<this->id()<<"]: "<<content<<endl;
}
