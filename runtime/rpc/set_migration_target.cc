#include "set_migration_target.h"

SetMigrationTarget::SetMigrationTarget(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output,struct rte_ring *rte_ring_request,struct rte_ring* rte_ring_reply,int worker_id)
	: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){
	tags.index=SETMIGRATIONTARGET;
	tags.tags=this;
	Proceed();
}

void SetMigrationTarget::Proceed() {
	if (status_ == CREATE) {
		status_ = PROCESS;
		service_->RequestSetMigrationTarget(&ctx_, &request_, &responder_, cq_, cq_,
																	 (void*)&tags);
	} else if (status_ == PROCESS) {
		new SetMigrationTarget(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		std::map<int, struct Local_view>::iterator it;
		std::cout<<"received a setmigrationtarget view request"<<std::endl;
		bool flag=true;
		int i;
		if(worker_id!=request_.migration_target_info().worker_id()){
			flag=false;
			reply_.set_fail_reason("Here is not the target you specified!");
		}else if(viewlist_input->size()!=request_.input_views_size()||viewlist_output->size()!=request_.output_views_size()){	 	 //check input and output size
			flag=false;
			std::cout<<"local inputsize:"<<viewlist_input->size()<<std::endl<<"request inputsize:"<<request_.input_views_size()<<std::endl<<"local outputsize:"<<viewlist_output->size()<<std::endl<<"local inputsize:"<<request_.output_views_size()<<std::endl;
			reply_.set_fail_reason("Input size or output size does not match!");
		}else{
			for(i=0;i<request_.input_views_size();i++){     //compare input
				if(viewlist_input->find(request_.input_views(i).worker_id())==viewlist_input->end()){
					flag=false;
					reply_.set_fail_reason("Input contents do not match!");
					break;
				}
			}
			for(i=0;i<request_.output_views_size();i++){     //compare output
				if(viewlist_output->find(request_.output_views(i).worker_id())==viewlist_output->end()){
					flag=false;
					reply_.set_fail_reason("Output contents do not match!");
					break;
				}
			}
		}
		if(flag==false){
			reply_.set_succeed(false);
			reply_.set_quota(0);
		}else{
			std::cout<<"setmigration target match succeed!"<<std::endl;
			reply_.set_succeed(true);
			reply_.set_quota(request_.quota());
			Local_view local_view;
			request_msg msg;
			std::map<int,Local_view> inputview;
			std::map<int,Local_view> outputview;
			msg.change_migration_msg_.input_views=&inputview;
			msg.change_migration_msg_.output_views=&outputview;
			void* rep_msg_ptr;
			int deque=1;
			msg.action=SETMIGRATIONTARGET;
			view_rpc2local(&msg.change_migration_msg_.migration_target_info,request_.migration_target_info());
			msg.change_migration_msg_.quota=request_.quota();
			for(i=0;i<request_.input_views_size();i++){     //add input to msg
				view_rpc2local(&local_view,request_.input_views(i));
				msg.change_migration_msg_.input_views->insert(std::make_pair(local_view.worker_id,local_view));
			}
			for(i=0;i<request_.output_views_size();i++){     //add output msg
				view_rpc2local(&local_view,request_.output_views(i));
				msg.change_migration_msg_.output_views->insert(std::make_pair(local_view.worker_id,local_view));
			}
			rte_ring_enqueue(rte_ring_request,&msg); //throw the msg to the ring
			while(1){
				sleep(2);
				//try to read the msg from the reply rte_ring
				deque=rte_ring_dequeue(rte_ring_reply,&rep_msg_ptr);
				struct reply_msg* rep_msg=static_cast<reply_msg*>(rep_msg_ptr);
				if(deque==0){
					std::cout<<"find reply"<<std::endl;
					if(rep_msg->reply){
						std::cout<<"Set migration target succeed!"<<std::endl;
					}
					break;
				}else{
					std::cout<<"empty reply queue"<<std::endl;
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

