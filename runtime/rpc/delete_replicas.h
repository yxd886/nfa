#ifndef DELETEreplicas
#define DELETEreplicas

#include "rpc_common.h"
class DeleteReplicas {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	DeleteReplicas(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output,struct rte_ring *rte_ring_request,struct rte_ring* rte_ring_reply,int worker_id,
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
	struct rte_ring *rte_ring_request;
	struct rte_ring *rte_ring_reply;
	int worker_id;
};

#endif
