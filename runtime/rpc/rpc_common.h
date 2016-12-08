#ifndef nfa_rpc_common
#define nfa_rpc_common


#include <memory>
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <map>
#include <queue>
#include <unistd.h>
#include <sys/shm.h>
#include <rte_common.h>
#include <rte_errno.h>
#include <rte_config.h>
#include <rte_ring.h>
#include <rte_malloc.h>
#include <rte_eal.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_lcore.h>

#include <grpc++/grpc++.h>

#include "../bessport/nfa_msg.grpc.pb.h"




using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using nfa_msg::LivenessRequest;
using nfa_msg::LivenessReply;
using nfa_msg::ViewList;
using nfa_msg::View;
using nfa_msg::CurrentView;
using nfa_msg::Runtime_RPC;
using nfa_msg::MigrationTarget;
using nfa_msg::MigrationNegotiationResult;
using nfa_msg::ReplicaList;
using nfa_msg::ReplicaNegotiationResult;
using nfa_msg::ReplicaInfo;
using nfa_msg::RecoverRuntimeResult;
using nfa_msg::RecoverRuntime;
using nfa_msg::RuntimeInfo;
using nfa_msg::RuntimeInfoRequest;
using nfa_msg::RuntimeStat;
using nfa_msg::RuntimeStatRequest;

#define	NUL 0
#define	SAYHELLO 1
#define	SAYHELLOAGAIN 2
#define	LIVENESSCHECK 3
#define	ADDOUTPUTVIEW 4
#define	ADDINPUTVIEW 5
#define	DELETEOUTPUTVIEW 6
#define	DELETEINPUTVIEW 7
#define	SETMIGRATIONTARGET 8
#define	ADDREPLICAS 9
#define	DELETEREPLICAS 10
#define	RECOVER 11
#define	QUERYRUNTIMEINFO 12
#define	QUERYRUNTIMESTAT 13




struct Local_view{
	uint64_t worker_id;
    char input_port_mac[6];
    char output_port_mac[6];
    char control_port_mac[6];
    char rpc_ip[20];
    uint64_t rpc_port;
};

typedef struct {
	int worker_id;
	char iport_mac[30];
	char oport_mac[30];
	char cport_mac[30];
	char rpc_ip[20];
	int rpc_port;
} cluster_view_msg;


typedef struct{
	Local_view migration_target_info;
	std::map<int, Local_view> *input_views;
	std::map<int, Local_view> *output_views;
	int quota;
}migration_target_msg;

typedef struct{
	Local_view replica;
	std::map<int, Local_view> *input_views;
	std::map<int, Local_view> *output_views;
}replica_msg;

typedef struct{
	int runtime_id;

}recover_msg;

typedef	RuntimeInfoRequest runtime_info_request;

typedef	RuntimeInfo runtime_info_msg;

typedef	RuntimeStatRequest runtime_stat_request;

typedef	RuntimeStat runtime_stat_msg;



struct  request_msg{
	int action;
	union{
		cluster_view_msg change_view_msg_;
		migration_target_msg change_migration_msg_;
		replica_msg change_replica_msg_;
		recover_msg set_recover_msg_;
		runtime_info_request *runtime_info_request_;
		runtime_stat_request *runtime_stat_request_;
	};

};

struct  reply_msg{
	int tag;
	int worker_id;
	bool reply;
	char fail_reason[80];
	runtime_info_msg *runtime_info_msg_;
	runtime_stat_msg *runtime_stat_msg_;
};




struct tag{
	int index;
	void* tags;

};


int parse_mac_addr(char *addr, const char *str );
int encode_mac_addr(char *str, char *addr);
int parse_ip_addr(char *addr, const char *str );
int encode_ip_addr( char *str ,char *addr );

void view_rpc2local(Local_view* local_view, View source);
void view_local2rpc(View* rpc_view_ptr, Local_view local_view );


#endif
