//
#include "coordinator.h"

struct migration_check{};
struct migratino_traffic_check {};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, broadcast_msg& x) {
  return f(meta::type_name("bmsg"), x.worker_ids, x.worker_control_ips, x.worker_control_macs, x.worker_states,
                                    x.worker_iport_macs, x.worker_oport_macs, x.vswitch_id);
}

behavior worker::make_behavior(){
#ifndef LOCAL_TEST
  // ignore master down
  // this->set_down_handler([=](const down_msg& dm) {
  //   aout(this) << "*** receive down msg" << endl;
  //   if (dm.source == actor_cast<strong_actor_ptr>(master_daemon_a_)) {
  //     aout(this) << "*** lost connection to server" << endl;
  //     destroy(master_daemon_a_);
  //     send(this, tick_atom::value);
  //     become(connecting());
  //   }
  // });

  // spawn the passive_acceptor first, the passive_acceptor will
  // publish a port to accept connections from other active_connector
  pa = spawn<passive_acceptor>(actor_cast<actor>(this), worker_id_, worker_control_ip, worker_control_port);
  this->send(pa, pa_start::value);
  return publishing();
#else
  if(is_recover_ == true){
    return worker_rejoin();
  }
  else{
    return running();
  }
#endif
}

behavior worker::publishing(){
  return{
    [=](pa_publish_ok){
      // passive_acceptor successfully binds to a port
      // now we launch the active connector

      ac = spawn<active_connector>(actor_cast<actor>(this), worker_id_);

      send(this, tick_atom::value);
      become(connecting());
    },
    [=](pa_publish_fail){
      quit();
    }
  };
}

behavior worker::connecting(){
 return{
   [=](tick_atom){
     // connector to the master daemon
     // once we finish connecting, we will start receiving the view_service message
     // sent from the master
     auto mm = system().middleman().actor_handle();
     this->request(mm, infinite, connect_atom::value, master_control_ip_, master_control_port_).await(
       [=](const node_id&, strong_actor_ptr serv, const std::set<std::string>& ifs){
         master_daemon_a_ = actor_cast<actor>(serv);
         this->monitor(master_daemon_a_);
         aout(this)<<"Successfully connected to master daemon "
                   <<master_control_ip_<<":"<<master_control_port_
                   <<endl;

         // here, depending on what the is_recover_
         // flag is set, we either enter worker_join
         // or enter worker_rejoin
         if(is_recover_==false){
           become(worker_join());
         }
         else{
           become(worker_rejoin());
         }
       },
       [=](const error& err) {
         aout(this)<<"connection error, reconnecting in 2s"<<endl;
         this->delayed_send(this, std::chrono::milliseconds(2000), tick_atom::value);
       }
     );
   },
   [=](view_service, vector<char>& serialized_broadcast_msg){
     // do nothing....
   }
 };
}

behavior worker::worker_join(){
  // just send a worker_join atom to the master and become running
  this->send(master_daemon_a_, worker_join::value,  worker_id_);
  return running();
}

behavior worker::worker_rejoin(){
  if(replication_strategy == 0){
    // if the replication_strategy is set to 0,
    // then we don't do replication at all, just
    // directly turn the state to running.
    this->send(master_daemon_a_, worker_rejoin::value,  worker_id_);
    this->send(master_daemon_a_, worker_recover_complete::value, worker_id_);
    return running();
  }
  else{
    // otherwise, we perform recovery.
    print("worker is restarted in rejoin behavior");
    replication_timer = std::chrono::system_clock::now();

    // first send the master_daemon_a the worker_rejoin atom.
    this->send(master_daemon_a_, worker_rejoin::value,  worker_id_);

    // then start sending worker_heart_beat.
    send(this, tick_atom::value);
    return {
      [=](tick_atom){
#ifndef LOCAL_TEST
        // send the worker_heart_beat message to indicate that the worker is alive
        this->send(master_daemon_a_, worker_heart_beat::value, worker_id_);
        this->delayed_send(this, std::chrono::milliseconds(100), tick_atom::value);
#endif
      },
      [=](int peer_worker_id, const actor& peer_worker, const actor& peer_replica_a, bool is_connection_normal){
        // we receive 2 actors and the connection status from peer worker peer_worker_id
        // the 2 actors are the peer_worker actors and the replica actors that the peer worker
        // created for us.

        auto outer_it = peer_map.find(peer_worker_id);
        if(outer_it!=peer_map.end()){
          string x;
          if(is_connection_normal){
            x = "normal";
          }
          else{
            x = "failed";
          }
          print("peer worker "+to_string(peer_worker_id)+" becomes "+x);


          outer_it->second.peer_worker_a = peer_worker;
          outer_it->second.peer_replica_a = peer_replica_a;
          outer_it->second.is_connection_normal = is_connection_normal;

          auto it = recovery_proxies.find(peer_worker_id);
          if(it!=recovery_proxies.end()){
            if(is_connection_normal){
              // we may generate several request_replica request....change this.
              // Notice: even if we may generate several request, but recovery_proxy will
              // quit after it receives the first message. This guarantees the correctness of the program.
              print("send request_replica msg to replication target runtime "+to_string(peer_worker_id));

              // we just send the corresponding recovery_proxy actor to the replica actor,
              // asking for the information stored on the replica actor.
              send(outer_it->second.peer_replica_a, retrieve_replica::value, it->second);
            }
            else{
              print("repliation target runtime "+to_string(peer_worker_id)+" fails");

              // if the peer worker storing the replica fails, we need to notify the
              // recovery_proxy.
              send(it->second, rep_peer_fail::value);
            }
          }
        }
      },
      [=](view_service, vector<char>& serialized_broadcast_msg){
        // the master will immediately broadcasts an old view message
        // saved previously to the recovered worker.
        // We use the first received broadcast message to determine
        // the number of replicas that we need to fetch in the cluster.

        binary_deserializer bd{system(), serialized_broadcast_msg};
        broadcast_msg bmsg;
        bd(bmsg);

        process_broadcast_msg(bmsg);

        if(met_first_view_msg == false){
          met_first_view_msg = true;
          // here we receive the first view message.
          // the first view meesage has already been processed in the
          // process_broadcast_msg function, and peer_map has already
          // contained all the peer workers in the cluster.
          // What we do here is that we create one recovery_proxy
          // for each peer worker, then the recovery_proxy can go fetch
          // the replica and perform replication by themselves.

          auto it = peer_map.begin();
          for(; it!=peer_map.end(); it++){
            if(it->first!=vswitch_id_){
              print("creating recovery_proxy actor to replication target rt "+to_string(it->first));

              // now create one recovery_proxy for each peer worker
              // the conditional expression guarantees that we don't
              // create recovery_proxy for virtual switch.
              auto recovery_proxy_a = spawn<recovery_proxy, detached>(hub,
                                                                      it->first,
                                                                      worker_id_,
                                                                      actor_cast<actor>(this),
                                                                      replication_strategy,
                                                                      output_queue,
                                                                      recover_queue);
              recovery_proxies.emplace(it->first, recovery_proxy_a);
            }
          }

          if(recovery_proxies.size()==0){
            recover_msg msg;
            msg.tag = RECOVER_FINISH_MSG;
            while(!recover_queue->try_enqueue(msg)){
            }

            this->send(master_daemon_a_, worker_recover_complete::value, worker_id_);
            become(running());
          }
        }
      },
      [=](recovery_proxy_complete, int replication_target_rt_id){
        // ok, the worker processing loop send a recovery_proxy_complete
        // message back to here. It means that the recovery_proxy for peer worker
        // replication_target_rt_id has finished recovering all of its flows and it quits.
        // And it sends a notification message to the packet processing loop. And then
        // the packet processing loop dilivers the message to the worker actor.
        // We just need to delelete the recovery_proxy from the unordered_map.

        auto it = recovery_proxies.find(replication_target_rt_id);
        if(it != recovery_proxies.end()){
          print("recovery from replication target runtime "+to_string(it->first)+" completes");
          recovery_proxies.erase(it->first);
          if(recovery_proxies.size()==0){
            // if all the recovery_proxies are deleted, then the recovery processs
            // finishes, We notifies the worker packet processing loop by enqueueing
            // a recover_finish_msg to the recovery_queue.

            recover_msg msg;
            msg.tag = RECOVER_FINISH_MSG;
            while(!recover_queue->try_enqueue(msg)){
            }

            for(auto inner_it = replication_nf_ec_helpers.begin(); inner_it!=replication_nf_ec_helpers.end(); inner_it++){
              unordered_map<actor_id, strong_actor_ptr>& umap = inner_it->second;
              peer_worker& pw = peer_map.find(inner_it->first)->second;

              if(pw.is_connection_normal==true){
                notify_replication_target_back_to_alive(umap, pw.peer_replica_a);
              }
            }

            this->send(master_daemon_a_, worker_recover_complete::value, worker_id_);
            auto completion_time = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(completion_time - replication_timer);
            aout(this)<<"[REPLICATION]: worker is recovered in "<<elapsed.count()<<"ms"<<endl;
            become(running());
          }
        }
      },
      [=](add_to_active_nf_ecs, actor_id id){
        // when the nf_execution_context is recovered, it will register with the worker

        print("actor "+to_string(worker_id_)+":"+to_string(id)+" becomes active");

        // we just add it to the active_nf_ecs map.
        active_nf_ecs.emplace(id, this->current_sender());
      },
      [=](request_replication_target, int replication_target_rt_id, actor_id id) -> result<int, actor, bool>{
        if(replication_target_rt_id != -1){
          replication_nf_ec_helpers.find(replication_target_rt_id)->second.emplace(id, this->current_sender());
          return result<int, actor, bool>(replication_target_rt_id, actor(unsafe_actor_handle_init), false);
        }
        else{
          return result<int, actor, bool>(-1, actor(unsafe_actor_handle_init), false);
        }
      }
    };
  }
}

behavior worker::running(){
  this->send(this, tick_atom::value);
  if(migration_strategy>0){
    delayed_send(this, std::chrono::seconds(1), migratino_traffic_check::value);
  }
  auto all_peer_ready = [=]()->bool{
    for(auto kv : peer_map){
      if(kv.second.is_connection_normal == false){
        return false;
      }
    }
    return true;
  };

  return{
    [=](tick_atom){
#ifndef LOCAL_TEST
      // send the worker_heart_beat message to indicate that the worker is alive
      this->send(master_daemon_a_, worker_heart_beat::value, worker_id_);
      this->delayed_send(this, std::chrono::milliseconds(100), tick_atom::value);
#endif
    },
    [=](worker_leave_start){
      // TODO: there's some bugs associated with the leave functionalities.
      // I don't need to solve it right now.
      if((peer_map.size()>0)&&all_peer_ready()){
#ifndef LOCAL_TEST
        this->send(master_daemon_a_, worker_leave::value, worker_id_);
#endif
      }
      else{
        delayed_send(this, std::chrono::milliseconds(100), worker_leave_start::value);
      }
    },
    [=](view_service, vector<char>& serialized_broadcast_msg){
      // process the view message.

      binary_deserializer bd{system(), serialized_broadcast_msg};
      broadcast_msg bmsg;
      bd(bmsg);

      process_broadcast_msg(bmsg);
    },
    [=](int peer_worker_id, const actor& peer_worker, const actor& peer_replica_a, bool is_connection_normal){
      auto outer_it = peer_map.find(peer_worker_id);
      if(outer_it!=peer_map.end()){
        string x;
        if(is_connection_normal){
          x = "normal";
        }
        else{
          x = "failed";
        }
        print("peer worker "+to_string(peer_worker_id)+" becomes "+x);


        outer_it->second.peer_worker_a = peer_worker;
        outer_it->second.peer_replica_a = peer_replica_a;
        outer_it->second.is_connection_normal = is_connection_normal;

        if(!is_connection_normal){
          // if the peer fails, we need to cancel some on-goning operations.

          if(peer_worker_id == vswitch_id_){

            // if the vswitch fails, we need to cancel
            // all the migration source, the migration source
            // are stored in migration_source_nf_ecs.
            auto it = migration_source_nf_ecs.begin();
            for(; it!=migration_source_nf_ecs.end(); it++){
              notify_migration_source_fail(it->second);
            }

            // if the vswitch fails, we also need to cancel
            // all the migration targets, the migration target
            // are stored in migration_target_nf_ecs.
            it = migration_target_nf_ecs.begin();
            for(; it!=migration_target_nf_ecs.end(); it++){
              notify_migration_target_fail(it->second);
            }
          }
          else{
            // if a peer worker fails, we notify:

            // first notify al the migration source to the peer worker
            auto it = migration_source_nf_ecs.find(peer_worker_id);
            notify_migration_source_fail(it->second);

            // then notify all the migration target from the peer worker
            it = migration_target_nf_ecs.find(peer_worker_id);
            notify_migration_target_fail(it->second);

            // finally, notify all the nf_ec_timers who is bond to the peer worker
            it = replication_nf_ec_helpers.find(peer_worker_id);
            notify_replication_target_fail(it->second);
          }
        }
        else{
          if(peer_worker_id!=vswitch_id_){
            // if the peer worker becomes normal again,
            // we notify all the nf_ec_timers that have already be bond to the peer worker
            // Then the nf_ec_timers can request a slot on the replica again.

            auto it = replication_nf_ec_helpers.find(peer_worker_id);
            notify_replication_target_back_to_alive(it->second, outer_it->second.peer_replica_a);
          }
        }
      }
    },
    [=](migratino_traffic_check){
      if(active_nf_ecs.size()>0){
        delayed_send(this, std::chrono::seconds(3), migration_check::value);
      }
      else{
        delayed_send(this, std::chrono::seconds(1), migratino_traffic_check::value);
      }
    },
    [=](migration_check){
      // do the fking migration here.
      // for demo purpose we only migration to worker 3
      if(peer_map.find(vswitch_id_)->second.is_connection_normal==true){
        auto vswitch_it = peer_map.find(vswitch_id_);

        // find a migration target using the rr_list
        auto mig_target_rt_it = peer_map.find(rr_list.front());

        // round rubin
        rr_list.push_front(rr_list.back());
        rr_list.pop_back();

        if((mig_target_rt_it!=peer_map.end())&&(mig_target_rt_it->second.is_connection_normal==true)){
          auto it = active_nf_ecs.begin();
          int migration_target_rt_id = mig_target_rt_it->first;

          while((it!=active_nf_ecs.end())&&(mig_source_counter<mig_batch)){
            // print("actor "+to_string(worker_id_)+":"+to_string(it->first)+" migrate to "+to_string(migration_target_rt_id));

            // here we decide to migrate active nf_execution_context to another worker in the cluster.
            // we send to the active nf_execution_context the following messages:
            // 1. start_migration atom
            // 2. the migration_target_rt_id
            // 3. the migration target worker actor
            // 4. the vswitch worker actor
            send(actor_cast<actor>(it->second),
                 start_migration::value,
                 migration_target_rt_id,
                 mig_target_rt_it->second.peer_worker_a,
                 vswitch_it->second.peer_worker_a);

            // after we have notified the nf_execution_context to migrate,
            // we first create a map record for the actor id of the nf_execution_context and
            // migration_target_rt_id
            migration_target_rt_record.emplace(it->first, migration_target_rt_id);

            // calculate migration time
            migration_time_map.emplace(it->first, std::chrono::system_clock::now());

            // then we create a map record for the actor id and the nf_execution_context actor.
            // and place the map record in migration_source_nf_ecs.
            migration_source_nf_ecs.find(migration_target_rt_id)->second.emplace(it->first, std::move(it->second));

            // possiblely increase some counters to limit the maximum outgoing migration
            mig_source_counter+=1;

            // remove the nf_execution_context actor id entry from active_nf_ecs.
            int key = it->first;
            it++;
            active_nf_ecs.erase(key);

            if(total_migrated_flow_counter == 0){
              migration_start_time = std::chrono::system_clock::now();
              total_migrated_flow_counter += 1;
            }
          }
        }
      }

      delayed_send(this, std::chrono::milliseconds(10), migration_check::value);
    },
    [=](create_migration_target_actor, vector<char>& flow_identifier, uint64_t service_chain_type_sig, int migration_source_rt_id,
        int replication_target_rt_id){
      if(peer_map.find(migration_source_rt_id)->second.is_connection_normal==false){
        // the migration source is dead out of some reason, respond error
        auto rp = make_response_promise();
        rp.deliver(nfactor_fail_atom::value);
      }
      else{

        // the worker receives the creae_migration_target_actor request
        // from another nf_execution_context within the cluster

        // first prepare a migration message that is going to send to the
        // packet processing loop
        mig_msg msg;

        // copy the flow_identifer to msg.flow_identifier
        // flow_identifier will be moved in the nf_execution_context constructor
        memcpy(msg.flow_identifier, flow_identifier.data(), 16);

        auto migration_target_a = this->spawn<nf_execution_context>(hub,
                                                                    worker_id_,
                                                                    flow_identifier,
                                                                    actor_cast<actor>(this),
                                                                    service_chain_type_sig,
                                                                    replication_strategy,
                                                                    migration_source_rt_id,
                                                                    peer_map.find(migration_source_rt_id)->second.peer_worker_a,
                                                                    peer_map.find(vswitch_id_)->second.peer_worker_a,
                                                                    replication_target_rt_id,
                                                                    output_queue);
        strong_actor_ptr* ptr = new strong_actor_ptr();
        *ptr = actor_cast<strong_actor_ptr>(migration_target_a);

        // store a strong_actor_pointer to the migration target actor
        // do reinterpret_cast to transform the pointer to void*
        msg.actor_strong_ptr = reinterpret_cast<void*>(ptr);

        // store the actor id of the migration target actor
        msg.aid = migration_target_a.id();

        // store the migration source rt id of the migration target actor
        msg.migration_source_rt_id = migration_source_rt_id;

        print("actor "+to_string(worker_id_)+":"+to_string(migration_target_a.id())+
              " migrate from "+to_string(migration_source_rt_id));

        // store the migration target actor in migration_target_nf_ecs.
        migration_target_nf_ecs.find(migration_source_rt_id)->second.emplace(migration_target_a.id(),
                                    actor_cast<strong_actor_ptr>(migration_target_a));

        // create response_promise for the migration target actor, remember that we
        // have not answered yet.
        migration_promises.emplace(migration_target_a.id(), make_response_promise());

        // finally enqueue the message to the mig queue
        while(!mig_queue->try_enqueue(msg)){
        }
      }
    },
    [=](uint64_t aid, int migration_source_rt_id){
      // we receive from the worker packet processing loop,
      // indicating that the migration target has been added to the hash table

      // first we retrieve the response promise associated with the migration target actor
      auto it = migration_promises.find(aid);

      // then we try to retrieve the migration_target_actor
      unordered_map<actor_id, strong_actor_ptr>& umap = migration_target_nf_ecs.find(migration_source_rt_id)->second;
      auto inner_it = umap.find(aid);

      if(inner_it != umap.end()){
        // here we still have the migration_target actor, it is not killed by the
        // some reasons. We deliver nfactor_ok_atom and the migration target actor
        // as the reponse back to the sender
        it->second.deliver(nfactor_ok_atom::value, actor_cast<actor>(inner_it->second));
      }

      // finally, the we need to erase the response promise
      migration_promises.erase(aid);
    },
    [=](request_vswitch_actor){
      // the nf_ec_timer may request vswitch_actor from the worker.

      auto vs_peer_worker = peer_map.find(vswitch_id_)->second;

      if(vs_peer_worker.is_connection_normal == false){
        // in case that the vswitch fails, we return an unsafe actor handle
        return actor(unsafe_actor_handle_init);
      }
      else{
        // otherwise we return the actor handle for the vswitch
        return vs_peer_worker.peer_worker_a;
      }
    },
    [=](add_to_active_nf_ecs, actor_id id){
      // when the nf_execution_context is created with is_registered flag set to true
      // it will register with the worker actor by sending actor_to_active_nf_ecs and its
      // actor_id.

      print("actor "+to_string(worker_id_)+":"+to_string(id)+" becomes active");

      // we just add it to the active_nf_ecs map.
      active_nf_ecs.emplace(id, this->current_sender());
    },
    [=](remove_from_active_nf_ecs, actor_id id){
      // when the nf_execution_context quits in normal processing mode
      // it send this message to unregister.

      print("actor "+to_string(worker_id_)+":"+to_string(id)+" is shutdown");

      auto it = active_nf_ecs.find(id);
      if(it != active_nf_ecs.end()){
        active_nf_ecs.erase(id);
      }
      else{
        // If we can't find the actor in the active_nf_ecs list
        // then it may be put to migration because this is a
        // concurrent process.
        // we check for the existance of whether the actor is migrated.
        // if so, we just delete it from the migration_source_nf_ecs map.
        auto inner_it = migration_target_rt_record.find(id);
        if(inner_it!=migration_target_rt_record.end()){
          migration_source_nf_ecs.find(migration_target_rt_record.find(id)->second)->second.erase(id);
          migration_target_rt_record.erase(id);
        }
      }
    },
    [=](remove_from_migration_source_nf_ecs, actor_id id, int migration_target_rt_id, bool to_active_nf_ecs){
      print("actor "+to_string(worker_id_)+":"+to_string(id)+
            " is no longer migration source to "+to_string(migration_target_rt_id));

      unordered_map<actor_id, strong_actor_ptr>& umap = migration_source_nf_ecs.find(migration_target_rt_id)->second;
      auto it = umap.find(id);

      if(it!=umap.end()){
        mig_source_counter -= 1;
        // if we can still find the migration source actor from the migration_source_nf_ecs.
        // then we try to remove it.

        if(to_active_nf_ecs){
          // if the to_active_nf_ecs is set to true, then it means that
          // then we need to add the actor back to active_nf_ecs again.
          // we directly move the strong_actor_ptr to active_nf_ecs.
          active_nf_ecs.emplace(id, std::move(it->second));
        }
        // we just erase the id from the following map.
        migration_target_rt_record.erase(id);
        umap.erase(id);

        if(to_active_nf_ecs==false){
          auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                                                     std::chrono::system_clock::now()-migration_time_map.find(id)->second);
          average_migration_completion_time += elapsed.count();

          total_finished_flow_counter += 1;
          if(total_finished_flow_counter == num_of_flow_to_mig){
            auto migration_end_time = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(migration_end_time - migration_start_time);
            long x = elapsed.count();
            aout(this)<<"[Migration finish]: "<<num_of_flow_to_mig<<" flows are migrated in "<<x<<"us"<<std::endl;
            aout(this)<<"[Migration finish]: The average flow migration time is "
                      <<(float(average_migration_completion_time)/float(num_of_flow_to_mig))<<"us"
                      <<std::endl;
          }
        }
        migration_time_map.erase(id);
      }
    },
    [=](remove_from_migration_target_nf_ecs, actor_id id, int migration_source_rt_id, bool to_active_nf_ecs){
      print("actor "+to_string(worker_id_)+":"+to_string(id)+
            " is no longer migration target from "+to_string(migration_source_rt_id));

      // all the operations here are similar with what is decribed in remove_from_migration_source_nf_ecs
      // message handler.

      unordered_map<actor_id, strong_actor_ptr>& umap = migration_target_nf_ecs.find(migration_source_rt_id)->second;
      auto it = umap.find(id);

      if(it!=umap.end()){
        if(to_active_nf_ecs){
          active_nf_ecs.emplace(id, std::move(it->second));
        }
        umap.erase(id);
      }
    },
    [=](request_replication_target, int replication_target_rt_id, actor_id id) -> result<int, actor, bool>{
      // the nf_ec_timer request a replication target(replica)

      if((replication_target_rt_id == -1) || (replication_target_rt_id == worker_id_)){
        // if the replication target runtime id of the nf_ec_timer is invalid,
        // we pick a new replication target runtime id for the nf_ec_timer
        // using a round rubin algorithm.

        // the rr_list is a round rubin list for selecting
        // the target, similar with the vswitch packet processing loop
        int rr_list_size = rr_list.size();
        for(int i=0; i<rr_list_size; i++){
          auto it = peer_map.find(rr_list.front());
          if(it->second.is_connection_normal == true){
            // we find a valid replication target

            print("replication helper "+to_string(worker_id_)+":"+to_string(id)+
                  " get replication target rt "+to_string(rr_list.front()));

            // we insert the nf_ec_timer to the replication_nf_ec_helpers.
            replication_nf_ec_helpers.find(rr_list.front())->second.emplace(id, this->current_sender());

            // we prepare the result first, note that
            // we can directly give the replica back to the nf_ec_timer
            result<int, actor, bool> r(rr_list.front(), it->second.peer_replica_a, true);

            // then we rotate the round-rubin list
            rr_list.push_front(rr_list.back());
            rr_list.pop_back();

            // return the result.
            return r;
          }
          else{
            // otherwise we just rotate the result
            rr_list.push_front(rr_list.back());
            rr_list.pop_back();
          }
        }

        print("replication helper "+to_string(worker_id_)+":"+to_string(id)+
              " do not get valid replication target rt");
        // if the code reaches here, then we can't find a valid replication target runtime
        // notifies the nf_ec_timer with a failure message.
        return result<int, actor, bool>(-1, actor(unsafe_actor_handle_init), false);
      }
      else{
        // if the nf_ec_timer has a valid replication target runtime id.
        // then we just give the replication target (replica back to it)

        auto it = peer_map.find(replication_target_rt_id);
        if(it->second.is_connection_normal == true){
          print("replication helper "+to_string(worker_id_)+":"+to_string(id)+
                " get replication target rt "+to_string(replication_target_rt_id));

          // the replica is available.
          // add the nf_ec_timer to replication_nf_ec_helepers and notify nf_ec_timer.
          replication_nf_ec_helpers.find(replication_target_rt_id)->second.emplace(id, this->current_sender());
          return result<int, actor, bool>(replication_target_rt_id, it->second.peer_replica_a, true);
        }
        else{
          print("replication helper "+to_string(worker_id_)+":"+to_string(id)+
                " do not get valid replication target rt");

          // the replica is not available, notify the nf_ec_timer
          // the nf_ec_timer will try again.
          replication_nf_ec_helpers.find(replication_target_rt_id)->second.emplace(id, this->current_sender());
          return result<int, actor, bool>(replication_target_rt_id, actor(unsafe_actor_handle_init), false);
        }
      }
    },
    [=](remove_from_replication_helpers, int replication_target_rt_id, actor_id id){
      print("replication helper "+to_string(worker_id_)+":"+to_string(id)+" is removed from replication helper list.");
      // remove the nf_ec_timer from the remove_from_replication_helpers.

      auto it = replication_nf_ec_helpers.find(replication_target_rt_id);
      if((it!=replication_nf_ec_helpers.end())&&(it->second.find(id)!=it->second.end())){
        it->second.erase(id);
      }
    }
  };
}

void worker::process_broadcast_msg(broadcast_msg& msg){
  vswitch_id_ = msg.vswitch_id;

  unordered_map<int, bool> record;
  for(auto kv : peer_map){
    record.emplace(kv.first, true);
  }

  for(size_t i=0; i<msg.worker_ids.size(); i++){

    int& peer_worker_id = msg.worker_ids[i];
    string& peer_control_ip = msg.worker_control_ips[i];
    string& peer_control_mac = msg.worker_control_macs[i];

    if((migration_source_nf_ecs.find(peer_worker_id) == migration_source_nf_ecs.end())&&
       (peer_worker_id != worker_id_) && (peer_worker_id != vswitch_id_)){
      // we receive a new peer, initialize the following maps
      // for this peer.

      unordered_map<actor_id, strong_actor_ptr> new_umap;
      migration_source_nf_ecs.emplace(peer_worker_id, new_umap);
      migration_target_nf_ecs.emplace(peer_worker_id, new_umap);
      replication_nf_ec_helpers.emplace(peer_worker_id, new_umap);
    }

    if(peer_map.find(peer_worker_id) == peer_map.end()){
      // we got a new peer
      if(peer_worker_id < worker_id_){
#ifndef LOCAL_TEST
        print("connect to peer worker "+to_string(peer_worker_id));
        // the peer is smaller than us, we should initiate the contact

        // we met a new peer worker, create a replica for this peer worker.
        auto new_replica_a = spawn<nf_replica, detached>(worker_id_, peer_worker_id);

        // send the created replica to the active conector
        this->send(ac, connect_to_peer::value, peer_worker_id, peer_control_ip,
                   worker_control_port, peer_control_mac, new_replica_a);
#endif
        // create a new peer_worker and put it in the map
        peer_worker new_p;
        peer_map.emplace(peer_worker_id, new_p);

        // create add a new element to the rr_list
        if(peer_worker_id!=vswitch_id_){
          rr_list.push_front(peer_worker_id);
        }
      }
      else if(peer_worker_id > worker_id_){
#ifndef LOCAL_TEST
        print("wait for connection from peer worker"+to_string(peer_worker_id));
        // the peer is larget than us, it initiates the connection to us

        // similar with the previous conditional clouse, except that
        // the new_replica_a is sent to passive acceptor.
        auto new_replica_a = spawn<nf_replica, detached>(worker_id_, peer_worker_id);

        this->send(pa, waiting_for_peer::value, peer_worker_id, peer_control_ip, peer_control_mac, new_replica_a);
#endif
        peer_worker new_p;
        peer_map.emplace(peer_worker_id, new_p);

        if(peer_worker_id!=vswitch_id_){
          rr_list.push_front(peer_worker_id);
        }
      }
      else{
        // do nothing
      }
    }
    else{
      record.erase(peer_worker_id);
    }
  }

  for(auto kv : record){
    int peer_worker_id = kv.first;
    if(peer_worker_id < worker_id_){
#ifndef LOCAL_TEST
      // the peer is smaller than us, we should initiate the contact

      // delete the replica for the peer worker
      this->send(ac, disconnect_peer::value, peer_worker_id);
#endif
      // delete the entry in the peer_map for the peer_worker_id
      peer_map.erase(peer_worker_id);

      // delete the elemenet for the peer_worker in the rr_list.
      auto it = std::find(rr_list.begin(), rr_list.end(), peer_worker_id);
      if(it!=rr_list.end()){
        rr_list.erase(it);
      }
    }
    else if(peer_worker_id > worker_id_){
#ifndef LOCAL_TEST
      // the peer is larget than us, it initiates the connection to us

      // similar as in the previous conditional clouse.

      // send(replicas.find(peer_worker_id)->second, replica_quit::value);
      this->send(pa, disconnect_peer::value, peer_worker_id);
#endif
      peer_map.erase(peer_worker_id);

      auto it = std::find(rr_list.begin(), rr_list.end(), peer_worker_id);
      if(it!=rr_list.end()){
        rr_list.erase(it);
      }
    }
    else{
      // do nothing
    }
  }
}

void worker::print(string msg){
  // aout(this)<<"INFO:[worker "<<worker_id_<<"]: "<<msg<<endl;
}
