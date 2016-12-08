#include "query_runtimestat.h"

QueryRuntimeStat::QueryRuntimeStat(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,struct rte_ring *rte_ring_request,struct rte_ring* rte_ring_reply,int worker_id)
	: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){
	// Invoke the serving logic right away.
	tags.index=QUERYRUNTIMESTAT;
	tags.tags=this;
	Proceed();
}

void QueryRuntimeStat::Proceed() {
	if (status_ == CREATE) {
		status_ = PROCESS;
		service_->RequestQueryRuntimeStat(&ctx_, &request_, &responder_, cq_, cq_,
																	 (void*)&tags);
	} else if (status_ == PROCESS) {
		new QueryRuntimeStat(service_, cq_,rte_ring_request,rte_ring_reply,worker_id);
		std::map<int, struct Local_view>::iterator it;

		bool ok_flag=false;
		int i;
			bool flag=true;
			if(worker_id!=request_.runtime_id()){
				reply_.set_fail_reason("this is not the runtime you are looking for");
				flag=false;
			}


			if(flag==false){
				std::cout<<"this is not the runtime you are looking for!"""<<std::endl;

			}else{

				request_msg msg;
				void* rep_msg_ptr;
				int deque=1;
				msg.action=QUERYRUNTIMESTAT;
				RuntimeStatRequest query_runtimestat;
				msg.runtime_stat_request_=&query_runtimestat;
				msg.runtime_stat_request_->CopyFrom(request_);
				rte_ring_enqueue(rte_ring_request,&msg); //throw the msg to the ring
				while(1){
					sleep(2);
					//try to read the msg from the reply rte_ring
					deque=rte_ring_dequeue(rte_ring_reply,&rep_msg_ptr);
					struct reply_msg* rep_msg=static_cast<reply_msg*>(rep_msg_ptr);
					if(deque==0){
						std::cout<<"find reply"<<std::endl;
						if(rep_msg->reply){
							reply_.CopyFrom(*(rep_msg->runtime_stat_msg_));
							ok_flag=true;
							std::cout<<"Runtime query succeed:"<<std::endl;
						}else{
							printf("%s\n",rep_msg->fail_reason);
						}
						break;
					}

				}
			}


			status_ = FINISH;
			responder_.Finish(reply_, Status::OK, (void*)&tags);

	} else {
		GPR_ASSERT(status_ == FINISH);
		delete this;
	}
}

