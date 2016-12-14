#include "nf_ec_timer.h"

#include <iostream>
using std::cout;
using std::endl;

using get_the_fking_replica = atom_constant<atom("gtfr")>;

nf_ec_timer::nf_ec_timer(
                         actor nf_ec,
                         actor worker_a,
                         int replication_target_rt_id,
                         int local_rt_id,
                         vector<char>& flow_identifier,
                         uint64_t service_chain_type_sig,
                         actor_id nf_ec_id) :

                           internal_pkt_counter_val(0),
                           repeat(0),
                           nf_ec(nf_ec),
                           worker_a(worker_a),
                           replication_target_rt_id(replication_target_rt_id),
                           replication_target_a(unsafe_actor_handle_init),
                           local_rt_id(local_rt_id),
                           flow_identifier(flow_identifier),
                           service_chain_type_sig(service_chain_type_sig),
                           pending_internal_transaction(false),
                           receive_fail_msg_before_replica_getter_finish(false),
                           vswitch_a(unsafe_actor_handle_init),
                           to_rt_id(-1),
                           clean_up_finish(true),
                           nf_ec_id(nf_ec_id),
                           quitting(false),
                           bond_to_replication_target_rt(false),
                           entry_setup(false){
	make_behavior();
}

void nf_ec_timer::make_behavior(){
  set_default_handler(print_and_drop);
}

void nf_ec_timer::handle_message(struct nf_ec_timer_quit*){
	state=NF_ec_timer_quit;
  if((clean_up_finish==true)&&(pending_internal_transaction==false)){

    if((replication_target_a != unsafe_actor_handle_init)&&(entry_setup == true)){
      // if we have a valid replica and we have already set up an entry on the
      // replica, just notify the replica to delete our entry.
      remote_send(replication_target_a, force_quit_atom::value, nf_ec_id);
      destroy(replication_target_a);
    }

    if(bond_to_replication_target_rt == true){
      // if we are bond to a replication_target_runtime,
      // notify the worker to remove us from that runtime
      local_send(worker_a, remove_from_replication_helpers::value, replication_target_rt_id, this->id());
    }

    destroy(worker_a);
    destroy(nf_ec);
    quit();
    print("quit");
  }
  else{
    quitting = true;
    delayed_send(this, std::chrono::milliseconds(500), nf_ec_timer_quit* value);
  }

}

void nf_ec_timer::handle_message(int new_replication_target_rt_id, const actor& new_replication_target_a){

	state=NEW_replication_target_rt_id;
  pending_internal_transaction = false;

  if(quitting == false){
    // the quitting flag is not set, we care about the result.

    if(new_replication_target_rt_id==-1){
      // we are not bond to a specific replica,
      // request replica to the worker again after 1s.

      print("Fail to acquire replication target runtime");

      local_delayed_send(this, std::chrono::milliseconds(3000+std::rand()%2000), prepare_to_get_replica::value);
    }
    else if(new_replication_target_a == unsafe_actor_handle_init){
      // we are bond to a replica, but the replica is failed
      // we only need to wait for notifications from the worker.

      print("The replica is placed on runtime "+to_string(new_replication_target_rt_id));

      bond_to_replication_target_rt = true;
      replication_target_rt_id = new_replication_target_rt_id;
    }
    else{
      print("The replica is placed on runtime "+to_string(new_replication_target_rt_id));

      // we are bond to a replica, and we receive a valid replica.
      // ask the replica to set up an entry for us.

      bond_to_replication_target_rt = true;
      replication_target_rt_id = new_replication_target_rt_id;
      replication_target_a = new_replication_target_a;
      struct get_the_fking_replica*get_the_fking_replica_value;
      local_send(this, get_the_fking_replica_value);
    }
  }
}




void nf_ec_timer::handle_message(struct prepare_to_get_replica*){
	state=PREPARE_to_get_replica;
  pending_internal_transaction = true;

  // ask the worker actor for replica
  local_send(worker_a, infinite, request_replication_target*value, replication_target_rt_id, this->id());

}


void nf_ec_timer::handle_message(struct nfactor_ok_atom*){
	switch(state){
	case GET_the_fking_replica:
    pending_internal_transaction = false;
    if(quitting == false){
      if(receive_fail_msg_before_replica_getter_finish == false){
        print("Correctly acquire replication target actor.");

        // we successfully set up an entry on the replica, notify the
        // nf_execution_context.
        entry_setup = true;
        remote_send(nf_ec, replication_target_rt_id, replication_target_a);
      }
      else{
        // The replica fails during the transaction, we can't trust the
        // received replica any more, just wait for an notification from
        // the worker to set up the entry again.

        print("Replication target runtime fails during transaction");
      }
    }
    break;
	case CHANGE_route_atom:
    destroy(vswitch_a);
    clean_up_finish = true;
    remote_send(nf_ec, clean_up_vswitch_table_finish::value);
    break;

	}
}

void nf_ec_timer::handle_message(const error& err ){
	switch(state){
	case GET_the_fking_replica:
    print("Fail to acquire replication target actor.");
    if(quitting == false){
      pending_internal_transaction = false;

      // we encounter errors when setting up the entry on the replica
      if(receive_fail_msg_before_replica_getter_finish == false){

        // if the replica is not failed during this time, we need to try again by
        // sending the get_the_fking_replica atom after 500ms.
        local_delayed_send(this, std::chrono::milliseconds(500), get_the_fking_replica::value);
      }
    }
    break;
	case CHANGE_route_atom:
    bool processed = false;
    auto m = err.context();

    message_handler mh;
    mh.assign(
      [&](nfactor_fail_atom){
        processed = true;
        destroy(vswitch_a);
        clean_up_finish = true;
        send(nf_ec, clean_up_vswitch_table_finish::value);
      }
    );
    m.apply(mh);

    if(processed==false){
      local_send(this, get_vswitch_atom::value);
    }
    break;
	case GET_vswitch_atom:
		local_delayed_send(this, std::chrono::milliseconds(1000), get_vswitch_atom::value);
		break;


	}

}

void nf_ec_timer::handle_message(struct get_the_fking_replica*){

	state=GET_the_fking_replica;
  if(pending_internal_transaction == true){
    // if there's ongoing transaction, retry after 500ms
    local_delayed_send(this, std::chrono::milliseconds(500), get_the_fking_replica::value);
  }
  else{
    if(entry_setup == false){
      // We only perform a retry if the entry is not set on the replica

      pending_internal_transaction = true;
      receive_fail_msg_before_replica_getter_finish = false;

      remote_send(replication_target_a, std::chrono::milliseconds(100), //100ms deadline
              create_new_replica*value, nf_ec_id, flow_identifier, service_chain_type_sig, 10);
    }
  }

}
void nf_ec_timer::handle_message(struct rep_peer_fail*){
	state=REP_peer_fail;
  print("the replication target runtime "+to_string(replication_target_rt_id)+" is failed");
  // the replica is failed

  if(pending_internal_transaction==true){
    // there's pending transaction, told the transaction
    // not to trust the result.
    receive_fail_msg_before_replica_getter_finish = true;
  }

  // clean up the replica
  destroy(replication_target_a);
  replication_target_a = actor(unsafe_actor_handle_init);
  entry_setup = false;

  if(quitting == false){
    // notify the nf_ec
  	remote_send(nf_ec, replication_target_rt_id, replication_target_a);
  }
}
void nf_ec_timer::handle_message(struct rep_peer_back_to_alive*, const actor& new_replication_target_a){

	state=REP_peer_back_to_alive;
  print("the replication target runtime "+to_string(replication_target_rt_id)+" is back to alive");
  // the replica is alive, set the replica, and retry setting up entry on replica

  replication_target_a = new_replication_target_a;

  if(quitting == false){
  	get_the_fking_replica*get_the_fking_replica_value;
    local_send(this, get_the_fking_replica_value);
  }
}
void nf_ec_timer::handle_message(struct clean_up_vswitch_table*, int arg_to_rt_id){
	state=CLEAN_up_vswitch_table;
  if(clean_up_finish == true){
    to_rt_id = arg_to_rt_id;
    clean_up_finish=false;
    local_send(this, get_vswitch_atom::value);
  }
}
void nf_ec_timer::handle_message(struct change_route_atom*){
	state=CHANGE_route_atom;
	remote_send(vswitch_a, std::chrono::milliseconds(50),
	                   forward_to_migration_target_actor::value,
	                   flow_identifier,
	                   to_rt_id);


}
void nf_ec_timer::handle_message(const actor& new_vswitch_a){
  if(new_vswitch_a == unsafe_actor_handle_init){
    local_delayed_send(this, std::chrono::milliseconds(1000), get_vswitch_atom::value);
  }
  else{
    vswitch_a = new_vswitch_a;
    local_send(this, change_route_atom::value);
  }
}

void nf_ec_timer::handle_message(struct get_vswitch_atom*){
	state=GET_vswitch_atom;
  local_send(worker_a, infinite, request_vswitch_actor::value);
}




inline void nf_ec_timer::print(string content){
  // aout(this)<<"INFO:[nf_ec_timer "<<local_rt_id<<":"<<this->id()<<"]: "<<content<<endl;
}
