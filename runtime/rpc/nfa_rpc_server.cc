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
using nfa_msg::AddOutputReply;
using nfa_msg::Runtime_RPC;
#include "concurrentqueue.h"
#include "nfa_rpc_server.h"


moodycamel::ConcurrentQueue<struct request_msg> rte_ring_request;
moodycamel::ConcurrentQueue<struct reply_msg> rte_ring_reply;
std::mutex mtx;

class ServerImpl final {
 public:
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

  class LivenessCheck {
     public:
      // Take in the "service" instance (in this case representing an asynchronous
      // server) and the completion queue "cq" used for asynchronous communication
      // with the gRPC runtime.
	  LivenessCheck(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,std::map< int, struct Local_view> viewlist_input,std::map< int, struct Local_view> viewlist_output)
          : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE){
        // Invoke the serving logic right away.
          tags.index=LIVENESSCHECK;
          tags.tags=this;
      	Proceed(viewlist_input,viewlist_output);
      }

      void Proceed(std::map< int, struct Local_view> viewlist_input,std::map< int ,struct Local_view> viewlist_output) {
        if (status_ == CREATE) {
          status_ = PROCESS;
          service_->RequestLivenessCheck(&ctx_, &request_, &responder_, cq_, cq_,
                                    (void*)&tags);
        } else if (status_ == PROCESS) {
          new LivenessCheck(service_, cq_,viewlist_input,viewlist_output);
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
    };



  class AddInputView {
      public:
       // Take in the "service" instance (in this case representing an asynchronous
       // server) and the completion queue "cq" used for asynchronous communication
       // with the gRPC runtime.
  	    AddInputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view> viewlist_input, std::map< int, struct Local_view> viewlist_output)
           : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
         // Invoke the serving logic right away.
           tags.index=ADDINPUTVIEW;
           tags.tags=this;
       	Proceed(viewlist_input,viewlist_output);
       }

       void Proceed(std::map< int, struct Local_view> viewlist_input,std::map< int, struct Local_view> viewlist_output) {
         if (status_ == CREATE) {
           status_ = PROCESS;
           service_->RequestAddInputView(&ctx_, &request_, &responder_, cq_, cq_,
                                     (void*)&tags);
         } else if (status_ == PROCESS) {
           new AddInputView(service_, cq_,viewlist_input,viewlist_output);
           std::map<int, struct Local_view>::iterator it;
           std::cout<<"received a addinput view request"<<std::endl;

          int i;

          for(i=0;i<request_.view_size();i++){
          	const View& outview=request_.view(i);
          		if(viewlist_input.find(outview.worker_id())!=viewlist_input.end()){
          			continue;
          		}else{
     					 bool deque=false;
     					 struct request_msg msg;
     					 struct reply_msg rep_msg;
     					 msg.tag=NFACTOR_CLUSTER_VIEW;
     					msg.action=ADDINPUTVIEW;
     					 msg.change_view_msg_.worker_id=outview.worker_id();
     					 msg.change_view_msg_.state=NFACTOR_WORKER_RUNNING;
     					 strcpy(msg.change_view_msg_.iport_mac,outview.input_port_mac().c_str());
     					 strcpy(msg.change_view_msg_.oport_mac,outview.output_port_mac().c_str());
     					 std::cout<<"throw the request to the ring"<<std::endl;
     					 rte_ring_request.enqueue(msg);
     					 std::cout<<"throw completed, waiting to read"<<std::endl;
     					 while(1){
     						 sleep(2);
     						 std::cout<<"get the lock to find reply"<<std::endl;
     						 deque=rte_ring_reply.try_dequeue(rep_msg);
     						 if(deque){
									 struct Local_view tmp;
									 std::cout<<"find reply"<<std::endl;
								   if(rep_msg.reply){
										 tmp.worker_id=outview.worker_id();
										 parse_mac_addr(tmp.control_port_mac,outview.control_port_mac().c_str());
										 parse_mac_addr(tmp.input_port_mac,outview.input_port_mac().c_str());
										 parse_mac_addr(tmp.output_port_mac,outview.output_port_mac().c_str());
										 parse_ip_addr(tmp.rpc_ip,outview.rpc_ip().c_str());
										 tmp.rpc_port=outview.rpc_port();
										 viewlist_output[tmp.worker_id]=tmp;
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
			   	char str_tmp[20];
			   	View * view_tmp=NULL;
				  	for(view_it=viewlist_output.begin();view_it!=viewlist_output.end();view_it++){

				  	  view_tmp=reply_.add_output_views();
						view_tmp->set_worker_id(view_it->first);
						encode_mac_addr(str_tmp,view_it->second.control_port_mac);
						view_tmp->set_control_port_mac(str_tmp);
						encode_mac_addr(str_tmp,view_it->second.input_port_mac);
						view_tmp->set_input_port_mac(str_tmp);
						encode_mac_addr(str_tmp,view_it->second.output_port_mac);
						view_tmp->set_output_port_mac(str_tmp);
						encode_ip_addr(str_tmp,view_it->second.rpc_ip);
						view_tmp->set_rpc_ip(str_tmp);
						view_tmp->set_rpc_port(view_it->second.rpc_port);

				  	}
				  	for(view_it=viewlist_input.begin();view_it!=viewlist_input.end();view_it++){

				  	  view_tmp=reply_.add_input_views();
						view_tmp->set_worker_id(view_it->first);
						encode_mac_addr(str_tmp,view_it->second.control_port_mac);
						view_tmp->set_control_port_mac(str_tmp);
						encode_mac_addr(str_tmp,view_it->second.input_port_mac);
						view_tmp->set_input_port_mac(str_tmp);
						encode_mac_addr(str_tmp,view_it->second.output_port_mac);
						view_tmp->set_output_port_mac(str_tmp);
						encode_ip_addr(str_tmp,view_it->second.rpc_ip);
						view_tmp->set_rpc_ip(str_tmp);
						view_tmp->set_rpc_port(view_it->second.rpc_port);

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
     };

  class AddOutputView {
        public:
         // Take in the "service" instance (in this case representing an asynchronous
         // server) and the completion queue "cq" used for asynchronous communication
         // with the gRPC runtime.
  	       AddOutputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view> viewlist_input, std::map< int, struct Local_view> viewlist_output)
             : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
           // Invoke the serving logic right away.
             tags.index=ADDOUTPUTVIEW;
             tags.tags=this;
         	Proceed(viewlist_input,viewlist_output);
         }

         void Proceed(std::map< int, struct Local_view> viewlist_input,std::map< int, struct Local_view> viewlist_output) {
           if (status_ == CREATE) {
             status_ = PROCESS;
             service_->RequestAddOutputView(&ctx_, &request_, &responder_, cq_, cq_,
                                       (void*)&tags);
           } else if (status_ == PROCESS) {
             new AddOutputView(service_, cq_,viewlist_input,viewlist_output);
             std::map<int, struct Local_view>::iterator it;
             std::cout<<"received a addoutput view request"<<std::endl;

            int i;

            for(i=0;i<request_.view_size();i++){
            	const View& outview=request_.view(i);
            		if(viewlist_output.find(outview.worker_id())!=viewlist_output.end()){
            			continue;
            		}else{
       					 bool deque=false;
       					 struct request_msg msg;
       					 struct reply_msg rep_msg;
       					 msg.tag=NFACTOR_CLUSTER_VIEW;
       					 msg.action=ADDOUTPUTVIEW;
       					 msg.change_view_msg_.worker_id=outview.worker_id();
       					 msg.change_view_msg_.state=NFACTOR_WORKER_RUNNING;
       					 strcpy(msg.change_view_msg_.iport_mac,outview.input_port_mac().c_str());
       					 strcpy(msg.change_view_msg_.oport_mac,outview.output_port_mac().c_str());
       					 std::cout<<"throw the request to the ring"<<std::endl;
       					 rte_ring_request.enqueue(msg);
       					 std::cout<<"throw completed, waiting to read"<<std::endl;
       					 while(1){
       						 sleep(2);
       						 std::cout<<"get the lock to find reply"<<std::endl;
       						 deque=rte_ring_reply.try_dequeue(rep_msg);
       						 if(deque){
       							 struct Local_view tmp;
       						   std::cout<<"find reply"<<std::endl;
       						   if(rep_msg.reply){
											 tmp.worker_id=outview.worker_id();
											 parse_mac_addr(tmp.control_port_mac,outview.control_port_mac().c_str());
											 parse_mac_addr(tmp.input_port_mac,outview.input_port_mac().c_str());
											 parse_mac_addr(tmp.output_port_mac,outview.output_port_mac().c_str());
											 parse_ip_addr(tmp.rpc_ip,outview.rpc_ip().c_str());
											 tmp.rpc_port=outview.rpc_port();
											 viewlist_output[tmp.worker_id]=tmp;
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
  				  	for(view_it=viewlist_output.begin();view_it!=viewlist_output.end();view_it++){

  				  	  view_tmp=reply_.add_output_views();
  						view_tmp->set_worker_id(view_it->first);
  						encode_mac_addr(str_tmp,view_it->second.control_port_mac);
  						view_tmp->set_control_port_mac(str_tmp);
  						encode_mac_addr(str_tmp,view_it->second.input_port_mac);
  						view_tmp->set_input_port_mac(str_tmp);
  						encode_mac_addr(str_tmp,view_it->second.output_port_mac);
  						view_tmp->set_output_port_mac(str_tmp);
  						encode_ip_addr(str_tmp,view_it->second.rpc_ip);
  						view_tmp->set_rpc_ip(str_tmp);
  						view_tmp->set_rpc_port(view_it->second.rpc_port);

  				  	}
  				  	for(view_it=viewlist_input.begin();view_it!=viewlist_input.end();view_it++){

  				  	  view_tmp=reply_.add_input_views();
  						view_tmp->set_worker_id(view_it->first);
  						encode_mac_addr(str_tmp,view_it->second.control_port_mac);
  						view_tmp->set_control_port_mac(str_tmp);
  						encode_mac_addr(str_tmp,view_it->second.input_port_mac);
  						view_tmp->set_input_port_mac(str_tmp);
  						encode_mac_addr(str_tmp,view_it->second.output_port_mac);
  						view_tmp->set_output_port_mac(str_tmp);
  						encode_ip_addr(str_tmp,view_it->second.rpc_ip);
  						view_tmp->set_rpc_ip(str_tmp);
  						view_tmp->set_rpc_port(view_it->second.rpc_port);

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
       };

  class DeleteOutputView {
         public:
          // Take in the "service" instance (in this case representing an asynchronous
          // server) and the completion queue "cq" used for asynchronous communication
          // with the gRPC runtime.
  	DeleteOutputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq, std::map< int ,struct Local_view> viewlist_input, std::map< int, struct Local_view> viewlist_output)
              : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
            // Invoke the serving logic right away.
              tags.index=DELETEOUTPUTVIEW;
              tags.tags=this;
          	Proceed(viewlist_input,viewlist_output);
          }

          void Proceed(std::map< int, struct Local_view> viewlist_input,std::map< int, struct Local_view> viewlist_output) {
            if (status_ == CREATE) {
              status_ = PROCESS;
              service_->RequestDeleteOutputView(&ctx_, &request_, &responder_, cq_, cq_,
                                        (void*)&tags);
            } else if (status_ == PROCESS) {
              new DeleteOutputView(service_, cq_,viewlist_input,viewlist_output);
              std::map<int, struct Local_view>::iterator it;
              std::cout<<"received a deleteoutput view request"<<std::endl;

             int i;

             for(i=0;i<request_.view_size();i++){
             	const View& outview=request_.view(i);
             		if((it=viewlist_output.find(outview.worker_id()))==viewlist_output.end()){
             			continue;
             		}else{
        					 bool deque=false;
        					 struct request_msg msg;
        					 struct reply_msg rep_msg;
        					 msg.tag=NFACTOR_CLUSTER_VIEW;
        					 msg.action=DELETEOUTPUTVIEW;
        					 msg.change_view_msg_.worker_id=outview.worker_id();
        					 msg.change_view_msg_.state=NFACTOR_WORKER_RUNNING;
        					 strcpy(msg.change_view_msg_.iport_mac,outview.input_port_mac().c_str());
        					 strcpy(msg.change_view_msg_.oport_mac,outview.output_port_mac().c_str());
        					 std::cout<<"throw the request to the ring"<<std::endl;
        					 rte_ring_request.enqueue(msg);
        					 std::cout<<"throw completed, waiting to read"<<std::endl;
        					 while(1){
        						 sleep(2);
        						 std::cout<<"get the lock to find reply"<<std::endl;
        						 deque=rte_ring_reply.try_dequeue(rep_msg);
        						 if(deque){
        							 struct Local_view tmp;
        						   std::cout<<"find reply"<<std::endl;
        						   if(rep_msg.reply){
        						  	 	 viewlist_output.erase(it);
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
   				  	for(view_it=viewlist_output.begin();view_it!=viewlist_output.end();view_it++){

   				  	  view_tmp=reply_.add_output_views();
   						view_tmp->set_worker_id(view_it->first);
   						encode_mac_addr(str_tmp,view_it->second.control_port_mac);
   						view_tmp->set_control_port_mac(str_tmp);
   						encode_mac_addr(str_tmp,view_it->second.input_port_mac);
   						view_tmp->set_input_port_mac(str_tmp);
   						encode_mac_addr(str_tmp,view_it->second.output_port_mac);
   						view_tmp->set_output_port_mac(str_tmp);
   						encode_ip_addr(str_tmp,view_it->second.rpc_ip);
   						view_tmp->set_rpc_ip(str_tmp);
   						view_tmp->set_rpc_port(view_it->second.rpc_port);

   				  	}
   				  	for(view_it=viewlist_input.begin();view_it!=viewlist_input.end();view_it++){

   				  	  view_tmp=reply_.add_input_views();
   						view_tmp->set_worker_id(view_it->first);
   						encode_mac_addr(str_tmp,view_it->second.control_port_mac);
   						view_tmp->set_control_port_mac(str_tmp);
   						encode_mac_addr(str_tmp,view_it->second.input_port_mac);
   						view_tmp->set_input_port_mac(str_tmp);
   						encode_mac_addr(str_tmp,view_it->second.output_port_mac);
   						view_tmp->set_output_port_mac(str_tmp);
   						encode_ip_addr(str_tmp,view_it->second.rpc_ip);
   						view_tmp->set_rpc_ip(str_tmp);
   						view_tmp->set_rpc_port(view_it->second.rpc_port);

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
        };


  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
   // new CallData(&service_, cq_.get());
   // new SayhelloAgain(&service_, cq_.get());
	  new LivenessCheck(&service_, cq_.get(),viewlist_input,viewlist_output);
	  new AddOutputView(&service_, cq_.get(),viewlist_input,viewlist_output);
	  new AddInputView(&service_, cq_.get(),viewlist_input,viewlist_output);
	  new DeleteOutputView(&service_, cq_.get(),viewlist_input,viewlist_output);
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
						static_cast<LivenessCheck *>(static_cast<struct tag*>(tag)->tags)->Proceed(viewlist_input,viewlist_output);
						break;
        case ADDOUTPUTVIEW:
						static_cast<AddOutputView *>(static_cast<struct tag*>(tag)->tags)->Proceed(viewlist_input,viewlist_output);
						break;
        case ADDINPUTVIEW:
						static_cast<AddInputView *>(static_cast<struct tag*>(tag)->tags)->Proceed(viewlist_input,viewlist_output);
						break;
        case DELETEOUTPUTVIEW:
        						static_cast<DeleteOutputView *>(static_cast<struct tag*>(tag)->tags)->Proceed(viewlist_input,viewlist_output);
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
};

void child(){
  std::cout<<"father process ok"<<std::endl;
  struct request_msg request;
  struct reply_msg reply;
  bool ok;
  while(1){

  		sleep(2);
	  ok=rte_ring_request.try_dequeue(request);
	  if(ok){
	  		switch(request.action){
	  			case ADDOUTPUTVIEW:
	  				//process of addoutputview
	  				break;
	  			case ADDINPUTVIEW:
	  				//process of addinputview
	  				break;
	  			default:
	  				break;
	  		}

	  	  reply.tag=request.tag;
	    	reply.worker_id=request.change_view_msg_.worker_id;
	    reply.reply=true;
	    std::cout<<"find request"<<std::endl;
	    rte_ring_reply.enqueue(reply);
	  }else{
	  			std::cout<<"empty request queue"<<std::endl;
	    }


	}
}

int main(int argc, char** argv) {
  ServerImpl server;
  std::thread t1(child);
	std::cout<<"Children process ok"<<std::endl;
  server.Run();
  return 0;
}
