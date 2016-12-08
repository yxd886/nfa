#include "liveness_check.h"



LivenessCheck::LivenessCheck(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,std::map< int, struct Local_view> *viewlist_input,std::map< int, struct Local_view> *viewlist_output, struct rte_ring *rte_ring_request,struct rte_ring *rte_ring_reply,int worker_id)
	: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){
	tags.index=LIVENESSCHECK;
	tags.tags=this;
	Proceed();
}

void LivenessCheck::Proceed() {
	if (status_ == CREATE) {
		status_ = PROCESS;
		service_->RequestLivenessCheck(&ctx_, &request_, &responder_, cq_, cq_,
				(void*)&tags);
	} else if (status_ == PROCESS) {
		new LivenessCheck(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		reply_.set_reply(true);//reply to the client

		status_ = FINISH;
		responder_.Finish(reply_, Status::OK, (void*)&tags);//check whether reply succeed
	} else {
		GPR_ASSERT(status_ == FINISH);
		delete this;
	}
}
