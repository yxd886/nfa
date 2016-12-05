/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <memory>
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <map>
#include <queue>
#include <unistd.h>
#include <sys/shm.h>

#include <grpc++/grpc++.h>

#include "nfa_msg.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using nfa_msg::LivenessRequest;
using nfa_msg::LivenessReply;
using nfa_msg::ViewList;
using nfa_msg::View;
using nfa_msg::CurrentView;
using nfa_msg::Runtime_RPC;
using nfa_msg::MigrationTarget;
using nfa_msg::MigrationNegotiationResult;
using nfa_msg::ReplicaList;
using nfa_msg::ReplicaNegotiationResult;
using nfa_msg::ReplicaInfo;
using nfa_msg::RecoverRuntimeResult;
using nfa_msg::RecoverRuntime;
using nfa_msg::RuntimeInfo;
using nfa_msg::RuntimeInfoRequest;
using nfa_msg::RuntimeStat;
using nfa_msg::RuntimeStatRequest;

#include "concurrentqueue.h"
#include "nfa_rpc_server.h"


class LivenessCheck {
public:
	// Take in the "service" instance (in this case representing an asynchronous
	// server) and the completion queue "cq" used for asynchronous communication
	// with the gRPC runtime.
	LivenessCheck(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,std::map< int, struct Local_view> *viewlist_input,std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg> *rte_ring_reply,int worker_id)
		: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){
		// Invoke the serving logic right away.
		tags.index=LIVENESSCHECK;
		tags.tags=this;
		Proceed();
	}

	void Proceed() {
		if (status_ == CREATE) {
			status_ = PROCESS;
			service_->RequestLivenessCheck(&ctx_, &request_, &responder_, cq_, cq_,
                                  (void*)&tags);
		} else if (status_ == PROCESS) {
			new LivenessCheck(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
			reply_.set_reply(true);

			status_ = FINISH;
			responder_.Finish(reply_, Status::OK, (void*)&tags);
		} else {
			GPR_ASSERT(status_ == FINISH);
			delete this;
		}
	}
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



class ServerImpl final {
public:
	ServerImpl(int worker_id,moodycamel::ConcurrentQueue<struct request_msg>* rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply)
		:worker_id(worker_id),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){

	}
	~ServerImpl() {
		server_->Shutdown();
		// Always shutdown the completion queue after the server.
		cq_->Shutdown();
	}

	// There is no shutdown handling in this code.
	void Run() {
		std::string server_address("0.0.0.0:50051");

		ServerBuilder builder;
		// Listen on the given address without any authentication mechanism.
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		// Register "service_" as the instance through which we'll communicate with
		// clients. In this case it corresponds to an *asynchronous* service.
		builder.RegisterService(&service_);
		// Get hold of the completion queue used for the asynchronous communication
		// with the gRPC runtime.
		cq_ = builder.AddCompletionQueue();
		// Finally assemble the server.
		server_ = builder.BuildAndStart();
		std::cout << "Server listening on " << server_address << std::endl;

		// Proceed to the server's main loop.
		HandleRpcs();
  }

private:
	// Class encompasing the state and logic needed to serve a request.



	class AddInputView {
	public:
		// Take in the "service" instance (in this case representing an asynchronous
		// server) and the completion queue "cq" used for asynchronous communication
		// with the gRPC runtime.
  	    AddInputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id)
			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply) {
  	    		// Invoke the serving logic right away.
  	    		tags.index=ADDINPUTVIEW;
  	    		tags.tags=this;
  	    		Proceed();
  	    }

       void Proceed() {
    	   	   if (status_ == CREATE) {
    	   		   status_ = PROCESS;
    	   		   service_->RequestAddInputView(&ctx_, &request_, &responder_, cq_, cq_,
                                     (void*)&tags);
    	   	   } else if (status_ == PROCESS) {
    	   		   new AddInputView(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
    	   		   std::map<int, struct Local_view>::iterator it;
    	   		   std::cout<<"received a addinput view request"<<std::endl;

    	   		   int i;

    	   		   for(i=0;i<request_.view_size();i++){
    	   			   const View& outview=request_.view(i);
    	   			   if(viewlist_input->find(outview.worker_id())!=viewlist_input->end()){
    	   				   continue;
    	   			   }else{
    	   				   bool deque=false;
    	   				   struct request_msg msg;
    	   				   struct reply_msg rep_msg;
    	   				   msg.action=ADDINPUTVIEW;
    	   				   msg.change_view_msg_.worker_id=outview.worker_id();
    	   				   strcpy(msg.change_view_msg_.iport_mac,outview.input_port_mac().c_str());
    	   				   strcpy(msg.change_view_msg_.oport_mac,outview.output_port_mac().c_str());
    	   				   std::cout<<"throw the request to the ring"<<std::endl;
    	   				   rte_ring_request->enqueue(msg);
    	   				   std::cout<<"throw completed, waiting to read"<<std::endl;
    	   				   while(1){
    	   					   sleep(2);
    	   					   std::cout<<"get the lock to find reply"<<std::endl;
    	   					   deque=rte_ring_reply->try_dequeue(rep_msg);
    	   					   if(deque){
    	   						   struct Local_view tmp;
    	   						   std::cout<<"find reply"<<std::endl;
    	   						   if(rep_msg.reply){
    	   							   view_rpc2local(&tmp,outview);
    	   							   viewlist_input->insert(std::make_pair(tmp.worker_id,tmp));
    	   						   }
    	   						   break;
    	   					   }else{
    	   						   std::cout<<"empty reply queue"<<std::endl;
    	   					   }

    	   				   }


    	   			   }

    	   		   }
    	   		   std::map<int , struct Local_view>::iterator view_it;
    	   		   //prepare CurrentView data to send back
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
		AddOutputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id)
    			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply) {
			// Invoke the serving logic right away.
             tags.index=ADDOUTPUTVIEW;
             tags.tags=this;
             Proceed();
		}

		void Proceed() {
			if (status_ == CREATE) {
				status_ = PROCESS;
				service_->RequestAddOutputView(&ctx_, &request_, &responder_, cq_, cq_,
                                       (void*)&tags);
			} else if (status_ == PROCESS) {
				new AddOutputView(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
				std::map<int, struct Local_view>::iterator it;
				std::cout<<"received a addoutput view request"<<std::endl;

				int i;

				for(i=0;i<request_.view_size();i++){
					const View& outview=request_.view(i);
					if(viewlist_output->find(outview.worker_id())!=viewlist_output->end()){
						continue;
					}else{
						bool deque=false;
						struct request_msg msg;
						struct reply_msg rep_msg;
						msg.action=ADDOUTPUTVIEW;
						msg.change_view_msg_.worker_id=outview.worker_id();
						strcpy(msg.change_view_msg_.iport_mac,outview.input_port_mac().c_str());
						strcpy(msg.change_view_msg_.oport_mac,outview.output_port_mac().c_str());
						std::cout<<"throw the request to the ring"<<std::endl;
						rte_ring_request->enqueue(msg);
						std::cout<<"throw completed, waiting to read"<<std::endl;
						while(1){
							sleep(2);
							std::cout<<"get the lock to find reply"<<std::endl;
							deque=rte_ring_reply->try_dequeue(rep_msg);
							if(deque){
								struct Local_view tmp;
								std::cout<<"find reply"<<std::endl;
								if(rep_msg.reply){
									view_rpc2local(&tmp,outview);
									viewlist_output->insert(std::make_pair(tmp.worker_id,tmp));
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
		DeleteOutputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id)
    			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply) {
			// Invoke the serving logic right away.
			tags.index=DELETEOUTPUTVIEW;
			tags.tags=this;
          	Proceed();
		}

		void Proceed() {
			if (status_ == CREATE) {
				status_ = PROCESS;
				service_->RequestDeleteOutputView(&ctx_, &request_, &responder_, cq_, cq_,
                                        (void*)&tags);
			} else if (status_ == PROCESS) {
				new DeleteOutputView(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
				std::map<int, struct Local_view>::iterator it;
				std::cout<<"received a deleteoutput view request"<<std::endl;

				int i;

				for(i=0;i<request_.view_size();i++){
					const View& outview=request_.view(i);
					if((it=viewlist_output->find(outview.worker_id()))==viewlist_output->end()){
						continue;
					}else{
						bool deque=false;
						struct request_msg msg;
						struct reply_msg rep_msg;
						msg.action=DELETEOUTPUTVIEW;
						msg.change_view_msg_.worker_id=outview.worker_id();
						strcpy(msg.change_view_msg_.iport_mac,outview.input_port_mac().c_str());
						strcpy(msg.change_view_msg_.oport_mac,outview.output_port_mac().c_str());
						std::cout<<"throw the request to the ring"<<std::endl;
						rte_ring_request->enqueue(msg);
						std::cout<<"throw completed, waiting to read"<<std::endl;
						while(1){
							sleep(2);
							std::cout<<"get the lock to find reply"<<std::endl;
							deque=rte_ring_reply->try_dequeue(rep_msg);
							if(deque){
								std::cout<<"find reply"<<std::endl;
								if(rep_msg.reply){
									viewlist_output->erase(it);
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
  		DeleteInputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output, moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id)
   			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply) {
  			// Invoke the serving logic right away.
  			tags.index=DELETEINPUTVIEW;
  			tags.tags=this;
  			Proceed();
  		}

  		void Proceed() {
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
  						bool deque=false;
  						struct request_msg msg;
  						struct reply_msg rep_msg;
  						msg.action=DELETEINPUTVIEW;
  						msg.change_view_msg_.worker_id=outview.worker_id();
  						strcpy(msg.change_view_msg_.iport_mac,outview.input_port_mac().c_str());
  						strcpy(msg.change_view_msg_.oport_mac,outview.output_port_mac().c_str());
  						std::cout<<"throw the request to the ring"<<std::endl;
  						rte_ring_request->enqueue(msg);
  						std::cout<<"throw completed, waiting to read"<<std::endl;
  						while(1){
  							sleep(2);
  							std::cout<<"get the lock to find reply"<<std::endl;
  							deque=rte_ring_reply->try_dequeue(rep_msg);
  							if(deque){
  								std::cout<<"find reply"<<std::endl;
  								if(rep_msg.reply){
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
		SetMigrationTarget(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view>* viewlist_input, std::map< int, struct Local_view> *viewlist_output,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id)
			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){
			// Invoke the serving logic right away.
			tags.index=SETMIGRATIONTARGET;
			tags.tags=this;
			Proceed();
		}

		void Proceed() {
			if (status_ == CREATE) {
				status_ = PROCESS;
				service_->RequestSetMigrationTarget(&ctx_, &request_, &responder_, cq_, cq_,
                                       (void*)&tags);
			} else if (status_ == PROCESS) {
				new SetMigrationTarget(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
				std::map<int, struct Local_view>::iterator it;
				std::cout<<"received a setmigrationtarget view request"<<std::endl;

				//compare received view with local view
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
					reply_msg rep_msg;
					bool deque;
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
					rte_ring_request->enqueue(msg); //throw the msg to the ring
					while(1){
						sleep(2);
						std::cout<<"get the lock to find reply"<<std::endl;
						deque=rte_ring_reply->try_dequeue(rep_msg);
						if(deque){
							std::cout<<"find reply"<<std::endl;
							if(rep_msg.reply){
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
				std::map< int, struct Local_view> * replicalist)
			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply),replicalist(replicalist){
			// Invoke the serving logic right away.
			tags.index=ADDREPLICAS;
			tags.tags=this;
			Proceed();
		}

		void Proceed() {
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
						reply_msg rep_msg;
						bool deque;

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
						rte_ring_request->enqueue(msg); //throw the msg to the ring
						while(1){
							sleep(2);
							std::cout<<"get the lock to find reply"<<std::endl;
							deque=rte_ring_reply->try_dequeue(rep_msg);
							if(deque){
								std::cout<<"find reply"<<std::endl;
								if(rep_msg.reply){
									ok_flag=true;
									std::cout<<"add replica "<<rep_msg.worker_id<<" succeed!"<<std::endl;
									replicalist->insert(std::make_pair(msg.change_replica_msg_.replica.worker_id,msg.change_replica_msg_.replica));
								}else{
									printf("%s\n",rep_msg.fail_reason);
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
				std::map< int, struct Local_view> * replicalist)
			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),viewlist_input(viewlist_input),viewlist_output(viewlist_output),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply),replicalist(replicalist){
			// Invoke the serving logic right away.
			tags.index=DELETEREPLICAS;
			tags.tags=this;
			Proceed();
		}

		void Proceed() {
			if (status_ == CREATE) {
				status_ = PROCESS;
				service_->RequestDeleteReplicas(&ctx_, &request_, &responder_, cq_, cq_,
                                       (void*)&tags);
			} else if (status_ == PROCESS) {
				new DeleteReplicas(service_, cq_,viewlist_input,viewlist_output,rte_ring_request,rte_ring_reply,worker_id,replicalist);
				std::map<int, struct Local_view>::iterator it;
				
				//compare received view with local view
				bool ok_flag=false;
				int i,j;
				for(j=0;j<request_.replicas_size();j++){
					bool flag=true;
					const ReplicaInfo& rpc_replica=request_.replicas(j);
					if(worker_id==rpc_replica.replica().worker_id()){
						//can not replica itself
						flag=false;
						reply_.set_fail_reason("The replica you want to delete is myself!");
					}else if(replicalist->find(rpc_replica.replica().worker_id())==replicalist->end()){
						//the replica does not exist
						flag=false;
						reply_.set_fail_reason("the replica does not exist!");

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
						reply_msg rep_msg;
						bool deque;

						msg.action=DELETEREPLICAS;


						view_rpc2local(&msg.change_replica_msg_.replica,rpc_replica.replica());
						for(i=0;i<rpc_replica.input_views_size();i++){     //add input to msg
							view_rpc2local(&local_view,rpc_replica.input_views(i));
							msg.change_replica_msg_.input_views->insert(std::make_pair(local_view.worker_id,local_view));
						}
						for(i=0;i<rpc_replica.output_views_size();i++){     //add output msg
							view_rpc2local(&local_view,rpc_replica.output_views(i));
							msg.change_replica_msg_.output_views->insert(std::make_pair(local_view.worker_id,local_view));
						}
						rte_ring_request->enqueue(msg); //throw the msg to the ring
						while(1){
							sleep(2);
							std::cout<<"get the lock to find reply"<<std::endl;
							deque=rte_ring_reply->try_dequeue(rep_msg);
							if(deque){
								std::cout<<"find reply"<<std::endl;
								if(rep_msg.reply){
									ok_flag=true;
									std::cout<<"delete replica "<<rep_msg.worker_id<<" succeed!"<<std::endl;
									replicalist->erase(replicalist->find(msg.change_replica_msg_.replica.worker_id));
								}else{
									printf("%s\n",rep_msg.fail_reason);
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
				std::map< int, struct Local_view> * replicalist)
			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply),replicalist(replicalist){
			// Invoke the serving logic right away.
			tags.index=RECOVER;
			tags.tags=this;
			Proceed();
		}

		void Proceed() {
			if (status_ == CREATE) {
				status_ = PROCESS;
				service_->RequestRecover(&ctx_, &request_, &responder_, cq_, cq_,
                                       (void*)&tags);
			} else if (status_ == PROCESS) {
				new Recover(service_, cq_,rte_ring_request,rte_ring_reply,worker_id,replicalist);
				std::map<int, struct Local_view>::iterator it;
				
				//compare received view with local view
				bool ok_flag=false;
				int i;
					bool flag=true;
					if(worker_id==request_.runtime_id()){
						//can not replica itself
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
						reply_msg rep_msg;
						bool deque;
						msg.action=RECOVER;
						msg.set_recover_msg_.runtime_id=request_.runtime_id();
						rte_ring_request->enqueue(msg); //throw the msg to the ring
						while(1){
							sleep(2);
							std::cout<<"get the lock to find reply"<<std::endl;
							deque=rte_ring_reply->try_dequeue(rep_msg);
							if(deque){
								std::cout<<"find reply"<<std::endl;
								if(rep_msg.reply){
									ok_flag=true;
									std::cout<<"recover succeed, start to replay runtime:"<<rep_msg.worker_id<<"'s function"<<std::endl;
									replicalist->erase(replicalist->find(msg.change_replica_msg_.replica.worker_id));
								}else{
									printf("%s\n",rep_msg.fail_reason);
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
		QueryRuntimeInfo(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id)
			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){
			// Invoke the serving logic right away.
			tags.index=QUERYRUNTIMEINFO;
			tags.tags=this;
			Proceed();
		}

		void Proceed() {
			if (status_ == CREATE) {
				status_ = PROCESS;
				service_->RequestQueryRuntimeInfo(&ctx_, &request_, &responder_, cq_, cq_,
                                       (void*)&tags);
			} else if (status_ == PROCESS) {
				new QueryRuntimeInfo(service_, cq_,rte_ring_request,rte_ring_reply,worker_id);
				std::map<int, struct Local_view>::iterator it;
				
				//compare received view with local view
				bool ok_flag=false;
				int i;
					bool flag=true;
					if(worker_id!=request_.runtime_id()){
						//can not replica itself
						reply_.set_fail_reason("this is not the runtime you are looking for");
						flag=false;
					}


					if(flag==false){
						std::cout<<"this is not the runtime you are looking for!"""<<std::endl;

					}else{

						request_msg msg;
						reply_msg rep_msg;
						bool deque;
						msg.action=QUERYRUNTIMEINFO;
						RuntimeInfoRequest query_runtimeinfo;
						msg.runtime_info_request_=&query_runtimeinfo;
						msg.runtime_info_request_->CopyFrom(request_);
						rte_ring_request->enqueue(msg); //throw the msg to the ring
						while(1){
							sleep(2);
							std::cout<<"get the lock to find reply"<<std::endl;
							deque=rte_ring_reply->try_dequeue(rep_msg);
							if(deque){
								std::cout<<"find reply"<<std::endl;
								if(rep_msg.reply){
									reply_.CopyFrom(*(rep_msg.runtime_info_msg_));
									ok_flag=true;
									std::cout<<"Runtime query succeed:"<<std::endl;
								}else{
									printf("%s\n",rep_msg.fail_reason);
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
		QueryRuntimeStat(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,moodycamel::ConcurrentQueue<struct request_msg> *rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply,int worker_id)
			: service_(service), cq_(cq), responder_(&ctx_), status_(CREATE),worker_id(worker_id),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){
			// Invoke the serving logic right away.
			tags.index=QUERYRUNTIMESTAT;
			tags.tags=this;
			Proceed();
		}

		void Proceed() {
			if (status_ == CREATE) {
				status_ = PROCESS;
				service_->RequestQueryRuntimeStat(&ctx_, &request_, &responder_, cq_, cq_,
                                       (void*)&tags);
			} else if (status_ == PROCESS) {
				new QueryRuntimeStat(service_, cq_,rte_ring_request,rte_ring_reply,worker_id);
				std::map<int, struct Local_view>::iterator it;
				
				//compare received view with local view
				bool ok_flag=false;
				int i;
					bool flag=true;
					if(worker_id!=request_.runtime_id()){
						//can not replica itself
						reply_.set_fail_reason("this is not the runtime you are looking for");
						flag=false;
					}


					if(flag==false){
						std::cout<<"this is not the runtime you are looking for!"""<<std::endl;

					}else{

						request_msg msg;
						reply_msg rep_msg;
						bool deque;
						msg.action=QUERYRUNTIMESTAT;
						RuntimeStatRequest query_runtimestat;
						msg.runtime_stat_request_=&query_runtimestat;
						msg.runtime_stat_request_->CopyFrom(request_);
						rte_ring_request->enqueue(msg); //throw the msg to the ring
						while(1){
							sleep(2);
							std::cout<<"get the lock to find reply"<<std::endl;
							deque=rte_ring_reply->try_dequeue(rep_msg);
							if(deque){
								std::cout<<"find reply"<<std::endl;
								if(rep_msg.reply){
									reply_.CopyFrom(*(rep_msg.runtime_stat_msg_));
									ok_flag=true;
									std::cout<<"Runtime query succeed:"<<std::endl;
								}else{
									printf("%s\n",rep_msg.fail_reason);
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



	// This can be run in multiple threads if needed.
	void HandleRpcs() {
		// Spawn a new CallData instance to serve new clients.
		// new CallData(&service_, cq_.get());
		// new SayhelloAgain(&service_, cq_.get());
		new LivenessCheck(&service_, cq_.get(),&viewlist_input,&viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		new AddOutputView(&service_, cq_.get(),&viewlist_input,&viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		new AddInputView(&service_, cq_.get(),&viewlist_input,&viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		new DeleteOutputView(&service_, cq_.get(),&viewlist_input,&viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		new DeleteInputView(&service_, cq_.get(),&viewlist_input,&viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		new SetMigrationTarget(&service_, cq_.get(),&viewlist_input,&viewlist_output,rte_ring_request,rte_ring_reply,worker_id);
		new AddReplicas(&service_, cq_.get(),&viewlist_input,&viewlist_output,rte_ring_request,rte_ring_reply,worker_id,&replicalist);
		new DeleteReplicas(&service_, cq_.get(),&viewlist_input,&viewlist_output,rte_ring_request,rte_ring_reply,worker_id,&replicalist);
		new Recover(&service_, cq_.get(),rte_ring_request,rte_ring_reply,worker_id,&replicalist);
		new QueryRuntimeInfo(&service_, cq_.get(),rte_ring_request,rte_ring_reply,worker_id);
		new QueryRuntimeStat(&service_, cq_.get(),rte_ring_request,rte_ring_reply,worker_id);

		void* tag;  // uniquely identifies a request.
		bool ok;
		while (true) {
			// Block waiting to read the next event from the completion queue. The
			// event is uniquely identified by its tag, which in this case is the
			// memory address of a CallData instance.
			// The return value of Next should always be checked. This return value
			// tells us whether there is any kind of event or cq_ is shutting down.
			GPR_ASSERT(cq_->Next(&tag, &ok));
			GPR_ASSERT(ok);
			switch (static_cast<struct tag*>(tag)->index){
			case LIVENESSCHECK:
				static_cast<LivenessCheck *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case ADDOUTPUTVIEW:
				static_cast<AddOutputView *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case ADDINPUTVIEW:
				static_cast<AddInputView *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case DELETEOUTPUTVIEW:
				static_cast<DeleteOutputView *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case DELETEINPUTVIEW:
				static_cast<DeleteInputView *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case SETMIGRATIONTARGET:
				static_cast<SetMigrationTarget *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case ADDREPLICAS:
				static_cast<AddReplicas *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case DELETEREPLICAS:
				static_cast<DeleteReplicas *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case RECOVER:
				static_cast<Recover *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case QUERYRUNTIMEINFO:
				static_cast<QueryRuntimeInfo *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			case QUERYRUNTIMESTAT:
				static_cast<QueryRuntimeStat *>(static_cast<struct tag*>(tag)->tags)->Proceed();
				break;
			default:
				break;

			}
		}
	}

	std::unique_ptr<ServerCompletionQueue> cq_;
	Runtime_RPC::AsyncService service_;
	std::unique_ptr<Server> server_;
	std::map<int , struct Local_view> viewlist_input;
	std::map< int, struct Local_view> viewlist_output;
	std::map< int, struct Local_view>  replicalist;
public:
	moodycamel::ConcurrentQueue<struct request_msg>* rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply;
	int worker_id;
};

void child(moodycamel::ConcurrentQueue<struct request_msg>* rte_ring_request,moodycamel::ConcurrentQueue<struct reply_msg>* rte_ring_reply){
	std::cout<<"father process ok"<<std::endl;
	struct request_msg request;
	struct reply_msg reply;
	RuntimeInfo runtimeinfo;
	RuntimeStat runtimestat;
	reply.runtime_info_msg_=&runtimeinfo;
	reply.runtime_stat_msg_=&runtimestat;
	bool ok;
	while(1){

		sleep(2);
		ok=rte_ring_request->try_dequeue(request);
		if(ok){
			switch(request.action){
				case ADDOUTPUTVIEW:
					//process of addoutputview
					reply.worker_id=request.change_view_msg_.worker_id;
					break;
				case ADDINPUTVIEW:
					//process of addinputview
					reply.worker_id=request.change_view_msg_.worker_id;
					break;
				case DELETEOUTPUTVIEW:
					//process of deleteoutputview
					reply.worker_id=request.change_view_msg_.worker_id;
					break;
				case DELETEINPUTVIEW:
					//process of deleteinputview
					reply.worker_id=request.change_view_msg_.worker_id;
					break;
				case SETMIGRATIONTARGET:
					//process of setmigrationtarget
					reply.worker_id=request.change_migration_msg_.migration_target_info.worker_id;
					break;
				case ADDREPLICAS:
					//process of add replicas
					reply.worker_id=request.change_replica_msg_.replica.worker_id;
					std::cout<<"add replica worker_id: "<<request.change_replica_msg_.replica.worker_id<<std::endl;
					break;
				case DELETEREPLICAS:
					//process of delete replicas
					reply.worker_id=request.change_replica_msg_.replica.worker_id;
					std::cout<<"delete replica worker_id: "<<request.change_replica_msg_.replica.worker_id<<std::endl;
					break;
				case RECOVER:
					//process of recover
					reply.worker_id=request.set_recover_msg_.runtime_id;
					std::cout<<"recover runtime worker_id: "<<request.change_replica_msg_.replica.worker_id<<std::endl;
					break;
				case QUERYRUNTIMEINFO:
					//process of QueryRuntimeInfo


					reply.runtime_info_msg_->set_succeed(true);

					break;
				case QUERYRUNTIMESTAT:
					//process of QueryRuntimeStat


					reply.runtime_stat_msg_->set_succeed(true);

					break;
				default:
					break;
			}

			reply.tag=request.action;

			reply.reply=true;
			std::cout<<"find request"<<std::endl;
			rte_ring_reply->enqueue(reply);
		}else{
			std::cout<<"empty request queue"<<std::endl;
		}


	}
}

int main(int argc, char** argv) {
	moodycamel::ConcurrentQueue<struct request_msg> rte_ring_request;
	moodycamel::ConcurrentQueue<struct reply_msg> rte_ring_reply;
	ServerImpl server(1,&rte_ring_request,&rte_ring_reply);
	std::thread t1(child,&rte_ring_request,&rte_ring_reply);
	std::cout<<"Children process ok"<<std::endl;
	server.Run();
	return 0;
}
