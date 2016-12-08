#include "recover.h"

Recover::Recover(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,struct rte_ring *rte_ring_request,struct rte_ring* rte_ring_reply,int worker_id,
		std::map< int, struct Local_view> * replicalist)
	: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply),replicalist(replicalist){
	tags.index=RECOVER;
	tags.tags=this;
	Proceed();
}

void Recover::Proceed() {
	if (status_ == CREATE) {
		status_ = PROCESS;
		service_->RequestRecover(&ctx_, &request_, &responder_, cq_, cq_,
																	 (void*)&tags);
	} else if (status_ == PROCESS) {
		new Recover(service_, cq_,rte_ring_request,rte_ring_reply,worker_id,replicalist);
		std::map<int, struct Local_view>::iterator it;
		bool ok_flag=false;
		int i;
			bool flag=true;
			if(worker_id==request_.runtime_id()){
				flag=false;
				reply_.set_fail_reason("the runtime you want to recover is  myself!");
			}else if(replicalist->find(request_.runtime_id())==replicalist->end()){
				//the replica does not exist
				flag=false;
				reply_.set_fail_reason("do not have the replica of the runtime that you want to recover!");

			}


			if(flag==false){
				std::cout<<reply_.fail_reason()<<std::endl;

			}else{

				request_msg msg;
				void* rep_msg_ptr;
				int deque=1;
				msg.action=RECOVER;
				msg.set_recover_msg_.runtime_id=request_.runtime_id();
				rte_ring_enqueue(rte_ring_request,&msg); //throw the msg to the ring
				while(1){
					sleep(2);
					//try to read the msg from the reply rte_ring
					deque=rte_ring_dequeue(rte_ring_reply,&rep_msg_ptr);
					struct reply_msg* rep_msg=static_cast<reply_msg*>(rep_msg_ptr);
					if(deque==0){
						std::cout<<"find reply"<<std::endl;
						if(rep_msg->reply){
							ok_flag=true;
							std::cout<<"recover succeed, start to replay runtime:"<<rep_msg->worker_id<<"'s function"<<std::endl;
							replicalist->erase(replicalist->find(msg.change_replica_msg_.replica.worker_id));
						}else{
							printf("%s\\n",rep_msg->fail_reason);
						}
						break;
					}else{
						std::cout<<"empty reply queue"<<std::endl;
					}

				}
			}

		if(ok_flag==false){
			reply_.set_ack(false);

		}else{
			reply_.set_ack(true);
		}

		status_ = FINISH;
		responder_.Finish(reply_, Status::OK, (void*)&tags);

	} else {
		GPR_ASSERT(status_ == FINISH);
		delete this;
	}
}
