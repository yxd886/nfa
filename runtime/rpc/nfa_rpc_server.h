// This is where we implement all the RPC calls.

// This grpc server implementation should use the async modle, according
// to the implementation in this grpc greeter_async_server.cc example.

// This is because we must provide our own thread implementation and bond the
// thread for handling RPC calls to a DPDK EAL thread. So that this thread
// can use the DPDK ring.

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




class LivenessCheck {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	LivenessCheck(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,std::map< int, struct Local_view> *viewlist_input,std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply,int worker_id);
	void Proceed();

private:

	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	LivenessRequest request_;
	LivenessReply reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<LivenessReply> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> *viewlist_input;
	std::map< int, struct Local_view> *viewlist_output;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class AddInputView {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	AddInputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id);

	void Proceed();

private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	ViewList request_;
	CurrentView reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<CurrentView> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> *viewlist_input;
	std::map< int, struct Local_view> *viewlist_output;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class AddOutputView {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	AddOutputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	ViewList request_;
	CurrentView reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<CurrentView> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> *viewlist_input;
	std::map< int, struct Local_view> *viewlist_output;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class DeleteOutputView {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	DeleteOutputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	ViewList request_;
	CurrentView reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<CurrentView> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> *viewlist_input;
	std::map< int, struct Local_view> *viewlist_output;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class DeleteInputView {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	DeleteInputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	ViewList request_;
	CurrentView reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<CurrentView> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> *viewlist_input;
	std::map< int, struct Local_view> *viewlist_output;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class SetMigrationTarget {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	SetMigrationTarget(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	MigrationTarget request_;
	MigrationNegotiationResult reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<MigrationNegotiationResult> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> *viewlist_input;
	std::map< int, struct Local_view> *viewlist_output;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class AddReplicas {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	AddReplicas(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id,
			std::map< int, struct Local_view> * replicalist);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	ReplicaList request_;
	ReplicaNegotiationResult reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<ReplicaNegotiationResult> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> *viewlist_input;
	std::map< int, struct Local_view> *viewlist_output;
	std::map< int, struct Local_view> * replicalist;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class DeleteReplicas {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	DeleteReplicas(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id,
			std::map< int, struct Local_view> * replicalist);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	ReplicaList request_;
	ReplicaNegotiationResult reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<ReplicaNegotiationResult> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> *viewlist_input;
	std::map< int, struct Local_view> *viewlist_output;
	std::map< int, struct Local_view> * replicalist;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class Recover {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	Recover(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id,
			std::map< int, struct Local_view> * replicalist);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	RecoverRuntime request_;
	RecoverRuntimeResult reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<RecoverRuntimeResult> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> * replicalist;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class QueryRuntimeInfo {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	QueryRuntimeInfo(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	RuntimeInfoRequest request_;
	RuntimeInfo reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<RuntimeInfo> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> * replicalist;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};

class QueryRuntimeStat {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	QueryRuntimeStat(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id);
	void Proceed();
private:
	Runtime_RPC::AsyncService* service_;
	ServerCompletionQueue* cq_;
	ServerContext ctx_;
	RuntimeStatRequest request_;
	RuntimeStat reply_;

	// The means to get back to the client.
	ServerAsyncResponseWriter<RuntimeStat> responder_;

	// Let's implement a tiny state machine with the following states.
	enum CallStatus { CREATE, PROCESS, FINISH };
	CallStatus status_;  // The current serving state.
	struct tag tags;
	std::map< int, struct Local_view> * replicalist;
	moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply;
	int worker_id;
};
#endif
