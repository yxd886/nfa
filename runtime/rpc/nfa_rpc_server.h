//
// This is where we implement all the RPC calls.

// This grpc server implementation should use the async modle, according
// to the implementation in this grpc greeter_async_server.cc example.

// This is because we must provide our own thread implementation and bond the
// thread for handling RPC calls to a DPDK EAL thread. So that this thread
// can use the DPDK ring.
#ifndef nfa_rpc_server
#define nfa_rpc_server


#define NFACTOR_WORKER_RUNNING 1
#define NFACTOR_WORKER_FAIL    2
#define NFACTOR_WORKER_LEAVE   3


#define  NUL 0
#define  SAYHELLO 1
#define SAYHELLOAGAIN 2
#define LIVENESSCHECK 3
#define ADDOUTPUTVIEW 4
#define ADDINPUTVIEW 5
#define DELETEOUTPUTVIEW 6
#define DELETEINPUTVIEW 7
#define SETMIGRATIONTARGET 8
#define  ADDREPLICAS 9




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
	char iport_mac[6];
	char oport_mac[6];
	char cport_mac[6];
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



struct  request_msg{
	int action;
	union{
		cluster_view_msg change_view_msg_;
		migration_target_msg change_migration_msg_;
		replica_msg change_replica_msg_;
	};

};

struct  reply_msg{
	int tag;
	int worker_id;
	bool reply;
	char fail_reason[80];
};




struct tag{
	int index;
	void* tags;

};



static int parse_mac_addr(char *addr, const char *str ){
	if (str != NULL && addr != NULL) {
		int r = sscanf(str,
				"%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
			       addr,
			       addr+1,
			       addr+2,
			       addr+3,
			       addr+4,
			       addr+5);

		if (r != 6)
			return -EINVAL;
	}

	return 0;
}
static int encode_mac_addr(char *str, char *addr  )
{
	if (str != NULL && addr != NULL) {
		int r = sprintf(str,
			       "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
			       addr,
			       addr+1,
			       addr+2,
			       addr+3,
			       addr+4,
			       addr+5);

		if (r != 6)
			return -EINVAL;
	}

	return 0;
}

static int parse_ip_addr(char *addr, const char *str )
{
	if (str != NULL && addr != NULL) {
		strcpy(addr,str);
	}

	return 0;
}

static int encode_ip_addr( char *str ,char *addr )
{
	if (str != NULL && addr != NULL) {
		strcpy(str,addr);
	}

	return 0;
}

static void view_rpc2local(Local_view* local_view, View source){
	local_view->worker_id=source.worker_id();
	parse_mac_addr(local_view->control_port_mac,source.control_port_mac().c_str());
	parse_mac_addr(local_view->input_port_mac,source.input_port_mac().c_str());
	parse_mac_addr(local_view->output_port_mac,source.output_port_mac().c_str());
	parse_ip_addr(local_view->rpc_ip,source.rpc_ip().c_str());
	local_view->rpc_port=source.rpc_port();
	}
static void view_local2rpc(View* rpc_view_ptr, Local_view local_view ){
	char str_tmp[20];
	rpc_view_ptr->set_worker_id(local_view.worker_id);
	encode_mac_addr(str_tmp,local_view.control_port_mac);
	rpc_view_ptr->set_control_port_mac(str_tmp);
	encode_mac_addr(str_tmp,local_view.input_port_mac);
	rpc_view_ptr->set_input_port_mac(str_tmp);
	encode_mac_addr(str_tmp,local_view.output_port_mac);
	rpc_view_ptr->set_output_port_mac(str_tmp);
	encode_ip_addr(str_tmp,local_view.rpc_ip);
	rpc_view_ptr->set_rpc_ip(str_tmp);
	rpc_view_ptr->set_rpc_port(local_view.rpc_port);
}



#endif
