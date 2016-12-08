#ifndef ADD_inputview
#define ADD_inputview

#include "rpc_common.h"

class AddInputView {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	AddInputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, struct rte_ring *rte_ring_request,struct rte_ring* rte_ring_reply,int worker_id);

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
	struct rte_ring *rte_ring_request;
	struct rte_ring *rte_ring_reply;
	int worker_id;
};

#endif
