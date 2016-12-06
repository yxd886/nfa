// This is the actor that handles flow packet processing.
// We should port the funcionality of nf_exeuction_context.cpp/hpp,
// to this file. 
#include "actor.h"


class nf_execution_context : public actor{
public:
  // this is used to launch migration source
  nf_execution_context(
                       network_function_hub& hub,
                       int local_rt_id,
                       vector<char>& flow_identifier,
                       const actor& worker_a,
                       service_chain_t service_chain_type_sig,
                       int replication_strategy,
                       worker_output_packet_queue* output_queue);

  // this is used to launch migration target
  nf_execution_context(
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
                       worker_output_packet_queue* output_queue);


  // this is used to launch recoverd migration source
  nf_execution_context(
                       network_function_hub& hub,
                       int local_rt_id,
                       vector<char>& flow_identifier,
                       const actor& worker_a,
                       service_chain_t service_chain_type_sig,
                       int replication_strategy,
                       // recover actor specific variable
                       vector<vector<char>>& p0_input_packet_bufs,
                       // vector<vector<char>>& p1_input_packet_bufs,
                       vector<char>& scs_buf,
                       worker_output_packet_queue* output_queue,
                       int replication_target_rt_id);


protected:
  behavior make_behavior();

private:
  /*********************private variables***************************************/

  //////common filed that must be correctly initialized during construction//////

  // basic nf_execution_context information
  network_function_hub& hub;
  int local_rt_id;
  vector<char> flow_identifier;
  actor worker_a;

  // the service chain type
  service_chain_t service_chain_type_sig;

  // whether enable replication or not
  int replication_strategy;

  starting_status s;

  ///////////////////////////////////////////////////////////////////////////////


  ///// filed that must be initialized for a migration target /////////////////

  // record the migration source
  int migration_source_rt_id;
  actor migration_source_a;

  // virtual switch actor
  actor my_vswitch_a;

  int replication_target_rt_id;

  ////////////////////////////////////////////////////////////////////////////////


  ///// filed that must be initialized for a recover actor       /////////////////

  vector<vector<char>> p0_input_packet_bufs;
  // vector<vector<char>> p1_input_packet_bufs;
  vector<char> scs_buf_;

  ////////////////////////////////////////////////////////////////////////////////


  bool pending_transaction;

  int migration_target_rt_id;
  int internal_pkt_counter;
  int retry_counter;

  size_t buffer_size;

  // keep track of the current behavior
  nf_ec_state cur_state;

  actor replication_target_a;
  actor migration_target_a;
  actor nf_ec_timer_a;

  vector<struct rte_mbuf*> p0_buffer;
  vector<struct rte_mbuf*> p1_buffer;

  vector<tuple<network_function*, unique_ptr<flow_state>>> service_chain;

  bool pending_clear_up_vswitch_table;

  bool is_registered;

  worker_output_packet_queue* output_queue;

  bool is_bond_to_replica;
  bool is_replica_alive;
  bool set_up_entry;

  /***************************useful internal atoms*******************************/

  using nf_ec_internal_action = atom_constant<atom("fkdo")>;

  using replica_internal_check = atom_constant<atom("cr")>;

  using try_acquire_migration_target_actor = atom_constant<atom("i0")>;

  using try_change_forwarding_path = atom_constant<atom("i1")>;

  using try_migrate_flow_state = atom_constant<atom("i2")>;

  /****************************flow finish quit function*************************/

  void flow_finish_quit(migration_status status);

  /*************************behaviors*******************************/
  // behaviors when started as migration source.
  behavior normal_run();
  behavior acquire_migration_target_actor(const actor& new_migration_target_rt_a, const actor& vswitch_a);
  behavior change_forwarding_path();
  behavior migrate_flow_state();

  // behaviors when started as migration target
  behavior wait_flow_states();

  /***********************helper functions****************************/
  // for replication
  vector<char> pkt_to_char_buf(struct rte_mbuf* pkt);

  // service chain processing
  bool servce_chain_process(struct rte_mbuf* input_pkt, bool from_p0);
  void process_pkt(struct rte_mbuf* input_pkt, bool from_p0);

  void source_clean_up(){
    send(nf_ec_timer_a, clean_up_vswitch_table::value, local_rt_id);
    pending_clear_up_vswitch_table = true;
    destroy(my_vswitch_a);
    destroy(migration_target_a);
    become(normal_run());
  }

  void target_clean_up(){
    send(nf_ec_timer_a, clean_up_vswitch_table::value, migration_source_rt_id);
    pending_clear_up_vswitch_table = true;
    destroy(my_vswitch_a);
    destroy(migration_source_a);
    flow_finish_quit(migration_status::in_migration_target);
  }

  void print_normal(string content);
  void print_migration_source(string content);
  void print_migration_target(string content);
  void print_replication(int new_replication_target_rt_id, const actor& new_replication_target_a);
  void print_recover(string content);
};
