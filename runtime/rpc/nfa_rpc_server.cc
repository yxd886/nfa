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

#include "nfa_rpc_server.h"


std::vector<struct vswitch_msg> rte_ring;


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
      	Proceed();
      }

      void Proceed() {
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
       	Proceed();
       }

       void Proceed() {
         if (status_ == CREATE) {
           status_ = PROCESS;
           service_->RequestAddOutputView(&ctx_, &request_, &responder_, cq_, cq_,
                                     (void*)&tags);
         } else if (status_ == PROCESS) {
           new AddOutputView(service_, cq_,viewlist);
           std::vector<Local_view>::iterator it;
           for(it=viewlist.begin();it!=viewlist.end();it++){
        	   if(request_.worker_id()==it->worker_id){
        		   reply_.set_reply(false);
        		   reply_.set_message("Areadly exists ");
        	   }

           }
           if(it==viewlist.end()){
               Local_view tmp;
               bool ok=false;
               struct vswitch_msg msg;
               msg.tag=NFACTOR_CLUSTER_VIEW;
               msg.msg_type=REQUEST;
               msg.reply_result=false;
               msg.change_view_msg_.worker_id=tmp.worker_id;
               msg.change_view_msg_.state=NFACTOR_WORKER_RUNNING;
               strcpy(msg.change_view_msg_.iport_mac,tmp.input_port_mac);
               strcpy(msg.change_view_msg_.oport_mac,tmp.output_port_mac);

               rte_ring.push_back(msg);

               std::vector<struct vswitch_msg>::iterator iter;
               while(1){


            	   for(iter=rte_ring.begin();iter!=rte_ring.end();iter++){
            		   if(iter->msg_type==REPLY&&iter->tag==NFACTOR_CLUSTER_VIEW&&iter->change_view_msg_.worker_id==msg.change_view_msg_.worker_id){
            			   break;
            		   }

            	   }
            	   if(iter==rte_ring.end()){
            		   //not find, loop again
            		   iter=rte_ring.begin();
            	   }else{
            		   //find.
            		   ok=iter->reply_result;
            		   rte_ring.erase(iter);
            		   break;

            	   }

               }

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
        	static_cast<LivenessCheck *>(static_cast<struct tag*>(tag)->tags)->Proceed();
        	break;
        case SAYHELLOAGAIN:
        //	static_cast<SayhelloAgain *>(static_cast<struct tag*>(tag)->tags)->Proceed();
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

int main(int argc, char** argv) {
  ServerImpl server;
  int pid;
  pid=fork();
  if(pid==0){
      server.Run();
  }else{

	  std::vector<struct vswitch_msg>::iterator iter;
	  while(1){


   	   for(iter=rte_ring.begin();iter!=rte_ring.end();iter++){
   		   if(iter->msg_type==REQUEST){
   			   break;
   		   }

   	   }
   	   if(iter==rte_ring.end()){
   		   //not find, loop again
   		   iter=rte_ring.begin();
   	   }else{
   		   //find.
   		   iter->reply_result=true;
   		   iter->msg_type=REPLY;

   	   }

      }

  }




  return 0;
}
