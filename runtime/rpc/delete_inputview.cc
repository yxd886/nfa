#include "delete_inputview.h"

DeleteInputView::DeleteInputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, struct rte_ring *rte_ring_request,struct rte_ring* rte_ring_reply,int worker_id)
	: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply) {
	tags.index=DELETEINPUTVIEW;
	tags.tags=this;
	Proceed();
}

void DeleteInputView::Proceed() {
	if (status_ == CREATE) {
		status_ = PROCESS;
		service_->RequestDeleteInputView(&ctx_, &request_, &responder_, cq_, cq_,
				(void*)&tags);
	} else if (status_ == PROCESS) {
		new DeleteInputView(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		std::map<int, struct Local_view>::iterator it;
		std::cout<<"received a deleteinput view request"<<std::endl;

		int i;

		for(i=0;i<request_.view_size();i++){
			const View& outview=request_.view(i);
			if((it=viewlist_input->find(outview.worker_id()))==viewlist_input->end()){
				continue;
			}else{
				int deque=1;
				struct request_msg msg;
				void* rep_msg_ptr;
				msg.action=DELETEINPUTVIEW;
				msg.change_view_msg_.worker_id=outview.worker_id();
				strcpy(msg.change_view_msg_.iport_mac,outview.input_port_mac().c_str());
				strcpy(msg.change_view_msg_.oport_mac,outview.output_port_mac().c_str());
				std::cout<<"throw the request to the ring"<<std::endl;
				//send the message to the request rte_ring
				rte_ring_enqueue(rte_ring_request,&msg);
				std::cout<<"throw completed, waiting to read"<<std::endl;
				while(1){
					sleep(2);
					//try to read the msg from the reply rte_ring
					deque=rte_ring_dequeue(rte_ring_reply,&rep_msg_ptr);
					struct reply_msg* rep_msg=static_cast<reply_msg*>(rep_msg_ptr);
					if(deque==0){
						std::cout<<"find reply"<<std::endl;
						if(rep_msg->reply){
							viewlist_input->erase(it);
						}
						break;
					}else{
						std::cout<<"empty reply queue"<<std::endl;
					}

				}


			}

		}
		std::map<int , struct Local_view>::iterator view_it;

		char str_tmp[20];
		View * view_tmp=NULL;
		for(view_it=viewlist_output->begin();view_it!=viewlist_output->end();view_it++){

			view_tmp=reply_.add_output_views();
			view_local2rpc(view_tmp,view_it->second);

		}
		for(view_it=viewlist_input->begin();view_it!=viewlist_input->end();view_it++){

			view_tmp=reply_.add_input_views();
			view_local2rpc(view_tmp,view_it->second);

		}
		status_ = FINISH;
		responder_.Finish(reply_, Status::OK, (void*)&tags);

	} else {
		GPR_ASSERT(status_ == FINISH);
		delete this;
	}
}
