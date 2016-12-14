#ifndef NF_EC_TIMER
#define NF_EC_TIMER

#define NF_ec_timer_quit 1
#define PREPARE_to_get_replica 2
#define	GET_the_fking_replica	3
#define	REP_peer_fail 4
#define	REP_peer_back_to_alive 5
#define	CLEAN_up_vswitch_table 6
#define CHANGE_route_atom 7
#define GET_vswitch_atom 8
#define	NEW_replication_target_rt_id 9


#include "atom_definition.h"
#include "actor.h"
#include <chrono>
#include <string>
#include <vector>

//using namespace caf;
using std::string;
using std::to_string;
using std::vector;

static constexpr int flow_timeout_time = 5; //5s flow timeout value.

class nf_ec_timer : public actor{
public:
  nf_ec_timer(
              actor nf_ec,
              actor worker_a,
              int replication_target_rt_id,
              int local_rt_id,
              vector<char>& flow_identifier,
              uint64_t service_chain_type_sig
             // actor_id nf_ec_id
							);

protected:
  void make_behavior();
  void handle_message(atom_type(msg_type::nf_ec_timer_quit));
  void handle_message(atom_type(msg_type::prepare_to_get_replica));
  void handle_message(atom_type(msg_type::get_the_fking_replica));
  void handle_message(atom_type(msg_type::rep_peer_fail));
  void handle_message(atom_type(msg_type::rep_peer_back_to_alive),const actor& new_replication_target_a);
  void handle_message(atom_type(msg_type::clean_up_vswitch_table),int arg_to_rt_id);
  void handle_message(atom_type(msg_type::change_route_atom));
  void handle_message(atom_type(msg_type::get_vswitch_atom));
  void handle_message(int new_replication_target_rt_id, const actor& new_replication_target_a);
  void handle_message(atom_type(msg_type::nfactor_ok_atom));
 // void handle_message(const error&);
  void handle_message(const actor& new_vswitch_a);



private:

  int internal_pkt_counter_val;
  int repeat;
  actor nf_ec;

  actor worker_a;
  int replication_target_rt_id;
  actor replication_target_a;
  int local_rt_id;
  vector<char> flow_identifier;
  uint64_t service_chain_type_sig;
  bool pending_internal_transaction;
  bool receive_fail_msg_before_replica_getter_finish;

  actor vswitch_a;
  int to_rt_id;
  bool clean_up_finish;

 // actor_id nf_ec_id;

  bool quitting;
  bool bond_to_replication_target_rt;
  bool entry_setup;
  int state;
  inline void print(string content);
};

#endif
