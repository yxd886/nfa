#ifndef ATOM_DEFINITION_HPP
#define ATOM_DEFINITION_HPP


// all the atom definition goes from here.

static constexpr int connection_timeout_s = 5;
static constexpr int connection_timeout_ms = 50;

/*****************************time out for migration and replication **********************/

static constexpr int migration_timeout_ms = 10;

static constexpr int replication_timeout_ms = 10;

struct nfactor_ok_atom 				{};
struct nfactor_fail_atom 			{};

struct worker_machine_check   {};

struct env_check_ok           {};
struct env_check_fail         {};
struct proxy_ok               {};
struct proxy_fail             {};


struct worker_ping            {};
struct worker_pong            {};

/***************** master_daemon to worker_machine_proxy ******************/

// request
struct wmp_start {};

// request
struct init_worker_env {};

// request
struct create_interface {};

// request
struct create_worker_container {};

// reqest
struct clean_up_worker_container {};

// request
struct check_no_need_to_restart {};

// request
struct restart_worker_machine {};

//request
struct check_boot_up {};

// universal success response
struct wmp_ok {};

// universal failure response
struct wmp_fail {};

// do we really need this?
struct wmp_terminate{};

/**************** interactions between master_daemon, worker_fsm and worker*****************************/

// check timeout for each handler
struct check_timeout {};

// worker successfully start and send worker_join
struct worker_join {};

// heartbeat atom sent by the worker
struct worker_heart_beat {};

// notify a worker to leave
struct worker_leave_start {};

// aknowledgement sent from a worker that finish all the leave procedure
struct worker_leave {};

// worker will sends a worker_rejoin message to master after recover boot up
struct worker_rejoin {};

// worker will send a worker_recover_complete message to master after
// it has successfully recovered.
struct worker_recover_complete {};

struct worker_fsm_quit {};

/**************notify master_daemon about wm_fsm_actor state ***************/
struct notify_wm_fsm_state {};

struct notify_worker_fsm_state {};

struct view_service {};


/**************worker notify nf_replica to respond to the nf_execution_context*******/
struct respond_to_nf_ec {};

struct retrieve_replica {};


/**************sent by the worker actor to nf actor *********************/

struct start_migration {};

struct create_migration_target_actor {};

struct forward_to_migration_target_actor {};

struct request_vswitch_actor {};

struct try_migrate_flow_state {};

struct migration_fail {};

struct rep_peer_fail {};

struct rep_peer_back_to_alive{};

struct migration_target_init {};

struct force_quit_atom {};

struct prepare_to_get_replica {};

struct request_replication_target {};

struct create_new_replica {};

struct clean_up_vswitch_table {};

struct clean_up_vswitch_table_finish {};

struct idle_checker {};

struct idle_kill   {};

struct nf_ec_timer_resume {};

struct nf_ec_timer_quit {};

struct nf_ec_timer_quit_finish {};

struct request_replica {};

struct request_replica_fail {};

struct recovery_proxy_complete {};


/*************************send from worker to worker_mesh_manager******************/

// for the active connector

struct start_connector {};

struct ask_for_worker {};

struct connect_to_peer {};

struct send_ac_ping {};

struct ac_ping {};

struct disconnect_peer {};

struct ac_quit {};

// for the passive accetpro

struct pa_pong{};

struct waiting_for_peer{};

struct pa_timeout_check {};

struct pa_quit{};

struct pa_publish_ok {};

struct pa_publish_fail{};

struct pa_start {};

struct resume_worker_connection{};

struct resume_worker_connection_ok{};

struct resume_worker_connection_fail {};

struct add_to_active_nf_ecs{};

struct remove_from_active_nf_ecs {};

struct  remove_from_migration_source_nf_ecs{};

struct remove_from_migration_target_nf_ecs{};

struct remove_from_replication_helpers {};

struct replica_quit {};

struct set_up_entry_ok_atom {};

struct nf_ec_timer_internal_atom {};
struct change_route_atom {};
struct get_vswitch_atom {};

#endif
