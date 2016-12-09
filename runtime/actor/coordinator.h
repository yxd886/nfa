// The coordinator actor should be implemented here.
#include "actor.h"

#include "atom_definition.h"

#include "cluster/passive_acceptor.hpp"
#include "cluster/active_connector.hpp"
#include "cluster/broadcast_msg.hpp"
#include "cluster/peer_worker.hpp"
#include "cluster/enable_local_test.hpp"

#include "actor/network_function_hub.hpp"
#include "actor/nf_execution_context.hpp"
#include "actor/nf_replica.hpp"
#include "actor/recovery_proxy.hpp"

#include "runtime/worker_output_packet_queue.hpp"
#include "runtime/worker_recovery_msg_queue.hpp"
#include "runtime/worker_mig_msg_queue.hpp"

#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <map>
#include <list>
#include <algorithm>

using namespace caf;
using std::string;
using std::vector;
using std::unordered_map;
using std::to_string;
using std::endl;
using std::map;
using std::list;

class worker : public event_based_actor{
public:
  worker(actor_config& config,
         const string& master_control_ip,
         uint16_t master_control_port,
         int worker_id,
         bool is_recover,
         const string& worker_control_ip,
         uint16_t worker_control_port,
         network_function_hub& hub,
         int replication_strategy,
         int migration_strategy,
         worker_output_packet_queue* output_queue,
         worker_recovery_msg_queue*  recover_queue,
         worker_mig_msg_queue*       mig_queue,
         int num_of_flow_to_mig,
         int mig_batch) :
           event_based_actor(config),
           master_control_ip_(master_control_ip),
           master_control_port_(master_control_port),
           master_daemon_a_(unsafe_actor_handle_init),
           worker_id_(worker_id),
           is_recover_(is_recover),
           worker_control_ip(worker_control_ip),
           worker_control_port(worker_control_port),
           vswitch_id_(-1),
           replication_strategy(replication_strategy),
           migration_strategy(migration_strategy),
           pa(unsafe_actor_handle_init),
           ac(unsafe_actor_handle_init),
           hub(hub),
           met_first_view_msg(false),
           output_queue(output_queue),
           recover_queue(recover_queue),
           mig_queue(mig_queue),
           num_of_flow_to_mig(num_of_flow_to_mig),
           mig_batch(mig_batch),
           mig_source_counter(0),
           total_migrated_flow_counter(0),
           total_finished_flow_counter(0),
           block(true),
           average_migration_completion_time(average_migration_completion_time){};

protected:
  behavior make_behavior();

private:
  string master_control_ip_;
  uint16_t master_control_port_;
  actor master_daemon_a_;
  int worker_id_;
  bool is_recover_;
  string worker_control_ip;
  uint16_t worker_control_port;
  int vswitch_id_;

  int replication_strategy;
  int migration_strategy;

  actor pa;
  actor ac;

  network_function_hub& hub;

  bool met_first_view_msg;

  worker_output_packet_queue* output_queue;
  worker_recovery_msg_queue*  recover_queue;
  worker_mig_msg_queue*       mig_queue;

  int num_of_flow_to_mig;
  int mig_batch;
  int mig_source_counter;

  int total_migrated_flow_counter;

  int total_finished_flow_counter;

  bool block;

  long average_migration_completion_time;

  std::chrono::system_clock::time_point migration_start_time;

  std::chrono::system_clock::time_point replication_timer;

  unordered_map<int, peer_worker> peer_map;

  list<int> rr_list;

  behavior publishing();

  behavior connecting();

  behavior worker_join();

  behavior worker_rejoin();

  behavior running();

  void process_broadcast_msg(broadcast_msg& msg);

  void print(string msg);

  unordered_map<actor_id, strong_actor_ptr> active_nf_ecs;
  unordered_map<actor_id, std::chrono::system_clock::time_point> migration_time_map;

  unordered_map<int, unordered_map<actor_id, strong_actor_ptr>> migration_source_nf_ecs;
  unordered_map<actor_id, int> migration_target_rt_record;


  unordered_map<int, unordered_map<actor_id, strong_actor_ptr>> migration_target_nf_ecs;

  unordered_map<int, unordered_map<actor_id, strong_actor_ptr>> replication_nf_ec_helpers;

  unordered_map<int, actor> recovery_proxies;

  unordered_map<actor_id, response_promise> migration_promises;

  inline void notify_migration_source_fail(unordered_map<actor_id, strong_actor_ptr> &umap){
    // send a migration_fail message to each migration source
    // stored in the umap.

    auto it = umap.begin();
    while(it!=umap.end()){
      send(actor_cast<actor>(it->second), migration_fail::value);
      active_nf_ecs.emplace(it->first, std::move(it->second));
      actor_id id = it->first;
      it++;
      umap.erase(id);
      migration_target_rt_record.erase(id);
    }
  }

  inline void notify_migration_target_fail(unordered_map<actor_id, strong_actor_ptr> &umap){
    // send migration_fail message to each migration target stored in the umap.

    auto it = umap.begin();
    while(it!=umap.end()){
      send(actor_cast<actor>(it->second), migration_fail::value);
      actor_id id = it->first;
      it++;
      umap.erase(id);
    }
  }

  inline void notify_replication_target_fail(unordered_map<actor_id, strong_actor_ptr> &umap){
    auto it = umap.begin();
    while(it!=umap.end()){
      send(actor_cast<actor>(it->second), rep_peer_fail::value);
      it++;
    }
  }

  inline void notify_replication_target_back_to_alive(unordered_map<actor_id, strong_actor_ptr> &umap,
                                                      const actor& replication_target_rt_a){
    auto it = umap.begin();
    while(it!=umap.end()){
      send(actor_cast<actor>(it->second), rep_peer_back_to_alive::value, replication_target_rt_a);
      it++;
    }
  }

};
