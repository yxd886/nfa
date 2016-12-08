#include "add_replicas.h"

AddReplicas::AddReplicas(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output,struct rte_ring *rte_ring_request,struct rte_ring* rte_ring_reply,int worker_id,
		std::map< int, struct Local_view> * replicalist)
	: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply),replicalist(replicalist){
	tags.index=ADDREPLICAS;
	tags.tags=this;
	Proceed();
}
void AddReplicas::Proceed() {
	if (status_ == CREATE) {
		status_ = PROCESS;
		service_->RequestAddReplicas(&ctx_, &request_, &responder_, cq_, cq_,
				(void*)&tags);
	} else if (status_ == PROCESS) {
		new AddReplicas(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id,replicalist);
		std::map<int, struct Local_view>::iterator it;
		std::cout<<"received a add replica request"<<std::endl;
		//compare received view with local view
		bool ok_flag=false;
		int i,j;
		for(j=0;j<request_.replicas_size();j++){
			bool flag=true;
			const ReplicaInfo& rpc_replica=request_.replicas(j);
			if(worker_id==rpc_replica.replica().worker_id()){
				//can not replica itself
				flag=false;
				reply_.set_fail_reason("The replica you want to add is myself!");
			}else if(replicalist->find(rpc_replica.replica().worker_id())!=replicalist->end()){
				//the replica already exists
				flag=false;
				reply_.set_fail_reason("the replica already exists!");

			}else if(viewlist_input->size()!=rpc_replica.input_views_size()||viewlist_output->size()!=rpc_replica.output_views_size()){	 	 //check input and output size
				flag=false;
				std::cout<<"local inputsize:"<<viewlist_input->size()<<std::endl<<"request inputsize:"<<rpc_replica.input_views_size()<<std::endl<<"local outputsize:"<<viewlist_output->size()<<std::endl<<"local inputsize:"<<rpc_replica.output_views_size()<<std::endl;
				reply_.set_fail_reason("Input size or output size does not match!");
			}else{
				for(i=0;i<rpc_replica.input_views_size();i++){     //compare input
					if(viewlist_input->find(rpc_replica.input_views(i).worker_id())==viewlist_input->end()){
						flag=false;
						reply_.set_fail_reason("Input contents do not match!");
						break;
					}
				}
				for(i=0;i<rpc_replica.output_views_size();i++){     //compare output
					if(viewlist_output->find(rpc_replica.output_views(i).worker_id())==viewlist_output->end()){
						flag=false;
						reply_.set_fail_reason("Output contents do not match!");
						break;
					}
				}
			}


			if(flag==false){
				std::cout<<reply_.fail_reason()<<std::endl;
				continue;

			}else{

				Local_view local_view;
				request_msg msg;
				std::map<int,Local_view> inputview;
				std::map<int,Local_view> outputview;
				msg.change_replica_msg_.input_views=&inputview;
				msg.change_replica_msg_.output_views=&outputview;
				void* rep_msg_ptr;
				int deque=1;

				msg.action=ADDREPLICAS;


				view_rpc2local(&msg.change_replica_msg_.replica,rpc_replica.replica());
				for(i=0;i<rpc_replica.input_views_size();i++){     //add input to msg
					view_rpc2local(&local_view,rpc_replica.input_views(i));
					msg.change_replica_msg_.input_views->insert(std::make_pair(local_view.worker_id,local_view));
				}
				for(i=0;i<rpc_replica.output_views_size();i++){     //add output msg
					view_rpc2local(&local_view,rpc_replica.output_views(i));
					msg.change_replica_msg_.output_views->insert(std::make_pair(local_view.worker_id,local_view));
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
							ok_flag=true;
							std::cout<<"add replica "<<rep_msg->worker_id<<" succeed!"<<std::endl;
							replicalist->insert(std::make_pair(msg.change_replica_msg_.replica.worker_id,msg.change_replica_msg_.replica));
						}else{
							printf("%s\\n",rep_msg->fail_reason);
						}
						break;
					}else{
						std::cout<<"empty reply queue"<<std::endl;
					}

				}
			}
		}
		if(ok_flag==false){
			reply_.set_succeed(false);

		}else{
			reply_.set_succeed(true);
		}
		std::map<int , struct Local_view>::iterator local_replica_it;
		//prepare replicalist data to send back
		View * view_tmp=NULL;
		for(local_replica_it=replicalist->begin();local_replica_it!=replicalist->end();local_replica_it++){

			view_tmp=reply_.add_current_replicas();
			view_local2rpc(view_tmp,local_replica_it->second);

		}

		status_ = FINISH;
		responder_.Finish(reply_, Status::OK, (void*)&tags);

	} else {
		GPR_ASSERT(status_ == FINISH);
		delete this;
	}
}
