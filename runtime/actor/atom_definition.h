#ifndef ATOM_DEFINITION_HPP
#define ATOM_DEFINITION_HPP

#define atom_type(A) different_atom_type<(int)A>


template <int>
class different_atom_type{
public:
	static different_atom_type value;
};
// all the atom definition goes from here.

static constexpr int connection_timeout_s = 5;
static constexpr int connection_timeout_ms = 50;

/*****************************time out for migration and replication **********************/

static constexpr int migration_timeout_ms = 10;

static constexpr int replication_timeout_ms = 10;

enum class msg_type{
	nfactor_ok_atom ,
	change_forwarding_path_nfactor_ok_atom,
	nfactor_fail_atom ,

	worker_machine_check,

	env_check_ok,
	env_check_fail,
	proxy_ok,
	proxy_fail,


	worker_ping,
	worker_pong,

	/***************** master_daemon to worker_machine_proxy ******************/

	// request
	wmp_start,

	// request
	init_worker_env,

	// request
	create_interface,

	// request
	create_worker_container,

	// reqest
	clean_up_worker_container,

	// request
	check_no_need_to_restart,

	// request
	restart_worker_machine,

	//request
	scheck_boot_up,

	// universal success response
	wmp_ok,

	// universal failure response
	wmp_fail,

	// do we really need this?
	wmp_terminate,

	/**************** interactions between master_daemon, worker_fsm and worker*****************************/

	// check timeout for each handler
	check_timeout,

	// worker successfully start and send worker_join
	worker_join,

	// heartbeat atom sent by the worker
	worker_heart_beat,

	// notify a worker to leave
	worker_leave_start,

	// aknowledgement sent from a worker that finish all the leave procedure
	worker_leave,

	// worker will sends a worker_rejoin message to master after recover boot up
	worker_rejoin,
	// worker will send a worker_recover_complete message to master after
	// it has successfully recovered.
	worker_recover_complete,

	worker_fsm_quit,

	/**************notify master_daemon about wm_fsm_actor state ***************/
	notify_wm_fsm_state,

	notify_worker_fsm_state,

	view_service,


	/**************worker notify nf_replica to respond to the nf_execution_context*******/
	respond_to_nf_ec,

	retrieve_replica,


	/**************sent by the worker actor to nf actor *********************/

	start_migration,

	create_migration_target_actor,

	forward_to_migration_target_actor,

	request_vswitch_actor,

	try_migrate_flow_state,

	migration_fail,


	rep_peer_fail,

	rep_peer_back_to_alive,

	migration_target_init,

	force_quit_atom,

	prepare_to_get_replica,

	request_replication_target,

	create_new_replica,

	clean_up_vswitch_table,

	clean_up_vswitch_table_finish,

	idle_checker,

	idle_kill,

	nf_ec_timer_resume,

	nf_ec_timer_quit,

	nf_ec_timer_quit_finish,

	request_replica,

	request_replica_fail,

	recovery_proxy_complete,


	/*************************send from worker to worker_mesh_manager******************/

	// for the active connector

	start_connector,

	ask_for_worker,

	connect_to_peer,

	send_ac_ping,

	ac_ping,

	disconnect_peer,

	ac_quit,

	// for the passive accetpro

	pa_pong,

	waiting_for_peer,

	pa_timeout_check,

	pa_quit,

	pa_publish_ok,

	pa_publish_fail,

	pa_start,

	resume_worker_connection,

	resume_worker_connection_ok,

	resume_worker_connection_fail,

	add_to_active_nf_ecs,

	remove_from_active_nf_ecs,

	remove_from_migration_source_nf_ecs,

	remove_from_migration_target_nf_ecs,

	remove_from_replication_helpers,

	replica_quit,

	set_up_entry_ok_atom,
	nf_ec_timer_internal_atom,

	change_route_atom,

	get_vswitch_atom,

  nf_ec_internal_action,

  replica_internal_check,

  try_acquire_migration_target_actor,

  try_change_forwarding_path,

	get_the_fking_replica
};






//


#endif
