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
#include <vector>
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
using nfa_msg::View;
using nfa_msg::AddOutputReply;
using nfa_msg::Runtime_RPC;
#include "concurrentqueue.h"
#include "nfa_rpc_server.h"


moodycamel::ConcurrentQueue<struct vswitch_msg> rte_ring_request;
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
	  LivenessCheck(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,std::vector< struct Local_view> viewlist)
          : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE){
        // Invoke the serving logic right away.
          tags.index=LIVENESSCHECK;
          tags.tags=this;
      	Proceed(viewlist);
      }

      void Proceed(std::vector< struct Local_view> viewlist) {
        if (status_ == CREATE) {
          status_ = PROCESS;
          service_->RequestLivenessCheck(&ctx_, &request_, &responder_, cq_, cq_,
                                    (void*)&tags);
        } else if (status_ == PROCESS) {
          new LivenessCheck(service_, cq_,viewlist);
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



  class AddOutputView {
      public:
       // Take in the "service" instance (in this case representing an asynchronous
       // server) and the completion queue "cq" used for asynchronous communication
       // with the gRPC runtime.
	  AddOutputView(Runtime_RPC::AsyncService* service, ServerCompletionQueue* cq,std::vector< struct Local_view> viewlist)
           : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
         // Invoke the serving logic right away.
           tags.index=ADDOUTPUTVIEW;
           tags.tags=this;
       	Proceed(viewlist);
       }

       void Proceed(std::vector< struct Local_view> viewlist) {
         if (status_ == CREATE) {
           status_ = PROCESS;
           service_->RequestAddOutputView(&ctx_, &request_, &responder_, cq_, cq_,
                                     (void*)&tags);
         } else if (status_ == PROCESS) {
           new AddOutputView(service_, cq_,viewlist);
           std::vector<Local_view>::iterator it;
           std::cout<<"received a addoutput view request"<<std::endl;
           for(it=viewlist.begin();it!=viewlist.end();it++){
        	   if(request_.worker_id()==it->worker_id){
        		   reply_.set_reply(false);
        		   reply_.set_message("Areadly exists ");
        	   }

           }
           if(it==viewlist.end()){
               Local_view tmp;
               bool ok=false;
               bool deque=false;
               struct vswitch_msg msg;
               struct reply_msg rep_msg;
               msg.tag=NFACTOR_CLUSTER_VIEW;
               msg.change_view_msg_.worker_id=request_.worker_id();
               msg.change_view_msg_.state=NFACTOR_WORKER_RUNNING;
               strcpy(msg.change_view_msg_.iport_mac,request_.input_port_mac().c_str());
               strcpy(msg.change_view_msg_.oport_mac,request_.output_port_mac().c_str());
               std::cout<<"throw the request to the ring"<<std::endl;


               rte_ring_request.enqueue(msg);
               std::cout<<"throw completed, waiting to read"<<std::endl;
               while(1){
            	   sleep(2);
            	   std::cout<<"get the lock to find reply"<<std::endl;
            	   deque=rte_ring_reply.try_dequeue(rep_msg);
            	   if(deque){
            		   std::cout<<"find reply"<<std::endl;
            		   ok=rep_msg.reply;
            		   break;
            	    }else{
            	    	std::cout<<"empty reply queue"<<std::endl;
            	    }

               }

               std::cout<<"readed it from the ring"<<std::endl;
               if(ok==true){
                   tmp.worker_id=request_.worker_id();
                   parse_mac_addr(tmp.control_port_mac,request_.control_port_mac().c_str());
                   parse_mac_addr(tmp.input_port_mac,request_.input_port_mac().c_str());
                   parse_mac_addr(tmp.output_port_mac,request_.output_port_mac().c_str());
                   parse_ip_addr(tmp.rpc_ip,request_.rpc_ip().c_str());
                   tmp.rpc_port=request_.rpc_port();
                   viewlist.push_back(tmp);
                   reply_.set_reply(true);
                   reply_.set_message("Add OutputView OK! ");
               }else{
                   reply_.set_reply(false);
                   reply_.set_message("Add OutputView Fail! ");
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
       View request_;
       AddOutputReply reply_;

       // The means to get back to the client.
       ServerAsyncResponseWriter<AddOutputReply> responder_;

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
	  new LivenessCheck(&service_, cq_.get(),viewlist);
	  new AddOutputView(&service_, cq_.get(),viewlist);
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
        	static_cast<LivenessCheck *>(static_cast<struct tag*>(tag)->tags)->Proceed(viewlist);
        	break;
        case ADDOUTPUTVIEW:
        	static_cast<AddOutputView *>(static_cast<struct tag*>(tag)->tags)->Proceed(viewlist);
        	break;
        default:
        	break;

      }
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  Runtime_RPC::AsyncService service_;
  std::unique_ptr<Server> server_;
  std::vector< struct Local_view> viewlist;
};

void child(){
  std::cout<<"father process ok"<<std::endl;
  struct vswitch_msg request;
  struct reply_msg reply;
  bool ok;
  while(1){

  		sleep(2);
	  ok=rte_ring_request.try_dequeue(request);
	  if(ok){
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
