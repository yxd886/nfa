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
	Local_view Migration_target_info;
	std::map<int, Local_view> input_views;
	std::map<int, Local_view> output_views;
	int quota;
}migration_target_msg;


#define NFACTOR_CLUSTER_VIEW 1
#define NFACTOR_CHANGE_ROUTE 2

#define REPLY 100
#define REQUEST 200


struct  request_msg{
  int action;
  union{
    cluster_view_msg change_view_msg_;
    migration_target_msg change_migration_msg_;
  };

};

struct  reply_msg{
  int tag;
  int worker_id;
  bool reply;
};




struct tag{
	int index;
	void* tags;

};



static int parse_mac_addr(char *addr, const char *str )
{
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

static void view_copy(Local_view* local_view, View source){
	local_view->worker_id=source.worker_id();
	parse_mac_addr(local_view->control_port_mac,source.control_port_mac().c_str());
	parse_mac_addr(local_view->input_port_mac,source.input_port_mac().c_str());
	parse_mac_addr(local_view->output_port_mac,source.output_port_mac().c_str());
	parse_ip_addr(local_view->rpc_ip,source.rpc_ip().c_str());
	local_view->rpc_port=source.rpc_port();
	}



#endif
