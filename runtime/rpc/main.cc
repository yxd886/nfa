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

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "nfa_msg.grpc.pb.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using nfa_msg::LivenessRequest;
using nfa_msg::LivenessReply;
using nfa_msg::View;
using nfa_msg::ViewList;
using nfa_msg::CurrentView;
using nfa_msg::MigrationTarget;
using nfa_msg::MigrationNegotiationResult;
using nfa_msg::ReplicaList;
using nfa_msg::ReplicaNegotiationResult;
using nfa_msg::ReplicaInfo;
using nfa_msg::Runtime_RPC;

class RuntimeClient {
public:
	explicit RuntimeClient(std::shared_ptr<Channel> channel)
      : stub_(Runtime_RPC::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
	bool LivenessCheck() {
		LivenessRequest request;
		LivenessReply reply;
		ClientContext context;
		CompletionQueue cq;

		Status status;

		std::unique_ptr<ClientAsyncResponseReader<LivenessReply> > rpc(
			stub_->AsyncLivenessCheck(&context, request, &cq));

		rpc->Finish(&reply, &status, (void*)1);
		void* got_tag;
		bool ok = false;

		GPR_ASSERT(cq.Next(&got_tag, &ok));

		GPR_ASSERT(got_tag == (void*)1);

		GPR_ASSERT(ok);

		if (status.ok()) {
			return reply.reply();

		} else {

			std::cout<<"RPC failed"<<std::endl;
			return false;
		}
	}
	bool AddOutputView(ViewList request) {
		CurrentView reply;
		ClientContext context;
		CompletionQueue cq;

		Status status;

		std::unique_ptr<ClientAsyncResponseReader<CurrentView> > rpc(
				stub_->AsyncAddOutputView(&context, request, &cq));

		rpc->Finish(&reply, &status, (void*)1);
		void* got_tag;
		bool ok = false;

		GPR_ASSERT(cq.Next(&got_tag, &ok));

		GPR_ASSERT(got_tag == (void*)1);

		GPR_ASSERT(ok);

		if (status.ok()) {
			return true;

		} else {
			std::cout<<"RPC failed"<<std::endl;
			return false;
		}
	}
	bool AddInputView(ViewList request) {
		CurrentView reply;
		ClientContext context;
		CompletionQueue cq;

		Status status;

		std::unique_ptr<ClientAsyncResponseReader<CurrentView> > rpc(
				stub_->AsyncAddInputView(&context, request, &cq));

		rpc->Finish(&reply, &status, (void*)1);
		void* got_tag;
		bool ok = false;

		GPR_ASSERT(cq.Next(&got_tag, &ok));

		GPR_ASSERT(got_tag == (void*)1);

		GPR_ASSERT(ok);

		if (status.ok()) {
			return true;

		} else {
			std::cout<<"RPC failed"<<std::endl;
			return false;
		}
	}

	bool DeleteOutputView(ViewList request) {
		CurrentView reply;
		ClientContext context;
		CompletionQueue cq;

		Status status;

		std::unique_ptr<ClientAsyncResponseReader<CurrentView> > rpc(
				stub_->AsyncDeleteOutputView(&context, request, &cq));

		rpc->Finish(&reply, &status, (void*)1);
		void* got_tag;
		bool ok = false;

		GPR_ASSERT(cq.Next(&got_tag, &ok));

		GPR_ASSERT(got_tag == (void*)1);

		GPR_ASSERT(ok);

		if (status.ok()) {
			return true;

		} else {
			std::cout<<"RPC failed"<<std::endl;
			return false;
		}
	}
	bool DeleteInputView(ViewList request) {
		CurrentView reply;
		ClientContext context;
		CompletionQueue cq;

		Status status;

		std::unique_ptr<ClientAsyncResponseReader<CurrentView> > rpc(
				stub_->AsyncDeleteInputView(&context, request, &cq));

		rpc->Finish(&reply, &status, (void*)1);
		void* got_tag;
		bool ok = false;

		GPR_ASSERT(cq.Next(&got_tag, &ok));

		GPR_ASSERT(got_tag == (void*)1);

		GPR_ASSERT(ok);

		if (status.ok()) {
			return true;

		} else {
			std::cout<<"RPC failed"<<std::endl;
			return false;
		}
	}
	bool SetMigrationTarget(MigrationTarget request) {
		MigrationNegotiationResult reply;
		ClientContext context;
		CompletionQueue cq;

		Status status;

		std::unique_ptr<ClientAsyncResponseReader<MigrationNegotiationResult> > rpc(
				stub_->AsyncSetMigrationTarget(&context, request, &cq));

		rpc->Finish(&reply, &status, (void*)1);
		void* got_tag;
		bool ok = false;

		GPR_ASSERT(cq.Next(&got_tag, &ok));

		GPR_ASSERT(got_tag == (void*)1);

		GPR_ASSERT(ok);

		if (status.ok()) {
			if(reply.succeed()){
				return true;
			}else{
				std::cout<<reply.fail_reason()<<std::endl;
			}


		} else {
			std::cout<<"RPC failed"<<std::endl;
			return false;
		}
	}
	bool AddReplicas(ReplicaList request) {
		ReplicaNegotiationResult reply;
		ClientContext context;
		CompletionQueue cq;

		Status status;

		std::unique_ptr<ClientAsyncResponseReader<ReplicaNegotiationResult> > rpc(
				stub_->AsyncAddReplicas(&context, request, &cq));

		rpc->Finish(&reply, &status, (void*)1);
		void* got_tag;
		bool ok = false;

		GPR_ASSERT(cq.Next(&got_tag, &ok));

		GPR_ASSERT(got_tag == (void*)1);

		GPR_ASSERT(ok);

		if (status.ok()) {
			if(reply.succeed()){
				return true;
			}else{
				std::cout<<reply.fail_reason()<<std::endl;
			}


		} else {
			std::cout<<"RPC failed"<<std::endl;
			return false;
		}
	}
private:
	// Out of the passed in Channel comes the stub, stored here, our view of the
	// server's exposed services.
	std::unique_ptr<Runtime_RPC::Stub> stub_;
};

int main(int argc, char** argv) {
	// Instantiate the client. It requires a channel, out of which the actual RPCs
	// are created. This channel models a connection to an endpoint (in this case,
	// localhost at port 50051). We indicate that the channel isn't authenticated
	// (use of InsecureChannelCredentials()).
	RuntimeClient nfa_rpc(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
	std::string user("world");
	bool reply ;

	reply= nfa_rpc.LivenessCheck();  // The actual RPC call!
	if(reply){
		std::cout << "Liveness Check: OK "<< std::endl;
	}else{
		std::cout << "Liveness Check: Fail "<< std::endl;
	}

	ViewList request;
	MigrationTarget migration_request;

	View* req=request.add_view();

	req->set_worker_id(2);
	req->set_input_port_mac("11:22:33:44:55:66");
	req->set_output_port_mac("22:33:44:55:66:77");
	req->set_control_port_mac("33:44:55:66:77:88");
	req->set_rpc_ip("192.168.1.1/30");
	req->set_rpc_port(80);
/*
	reply = nfa_rpc.AddOutputView(request);
	if(reply){
		std::cout << "AddOutputView: OK "<< std::endl;
	}else{
		std::cout << "AddOutputView: Fail "<< std::endl;
	}


	reply = nfa_rpc.DeleteOutputView(request);

    if(reply){
  	  std::cout << "DeleteOutputView: OK "<< std::endl;
    }else{
  	  std::cout << "DeleteOutputView: Fail "<< std::endl;
    }

  */
	reply = nfa_rpc.AddInputView(request);

	if(reply){
		std::cout << "AddInputView: OK "<< std::endl;
	}else{
		std::cout << "AddInputView: Fail "<< std::endl;
	}
/*
    reply = nfa_rpc.DeleteInputView(request);

	if(reply){
    		std::cout << "DeleteInputView: OK "<< std::endl;
    	}else{
    		std::cout << "DeleteInputView: Fail "<< std::endl;
    }


*/
	migration_request.mutable_migration_target_info()->set_worker_id(1);
	migration_request.mutable_migration_target_info()->set_input_port_mac("11:22:33:44:55:66");
	migration_request.mutable_migration_target_info()->set_output_port_mac("22:33:44:55:66:77");
	migration_request.mutable_migration_target_info()->set_control_port_mac("33:44:55:66:77:88");
	migration_request.mutable_migration_target_info()->set_rpc_ip("192.168.1.1/30");
	migration_request.mutable_migration_target_info()->set_rpc_port(80);
	req=migration_request.add_input_views();
	req->set_worker_id(2);
	req->set_input_port_mac("11:22:33:44:55:66");
	req->set_output_port_mac("22:33:44:55:66:77");
	req->set_control_port_mac("33:44:55:66:77:88");
	req->set_rpc_ip("192.168.1.1/30");
	req->set_rpc_port(80);

	reply = nfa_rpc.SetMigrationTarget(migration_request);

	if(reply){
		std::cout << "SetMigrationTarget: OK "<< std::endl;
	}else{
		std::cout << "SetMigrationTarget: Fail "<< std::endl;
	}

	ReplicaList replicalist_request;
	ReplicaInfo* replica_info=replicalist_request.add_replicas();
/*
	replica_info.mutable_replica()->set_worker_id(3);
	replica_info.mutable_replica()->set_input_port_mac("11:22:33:44:55:66");
	replica_info.mutable_replica()->set_output_port_mac("22:33:44:55:66:77");
	replica_info.mutable_replica()->set_control_port_mac("33:44:55:66:77:88");
	replica_info.mutable_replica()->set_rpc_ip("192.168.1.1/30");
	replica_info.mutable_replica()->set_rpc_port(80);
	*/
	replica_info->mutable_replica()->CopyFrom(migration_request.migration_target_info());
	replica_info->mutable_replica()->set_worker_id(3);


	req=replica_info->add_input_views();
	req->CopyFrom(migration_request.input_views(0));
	/*
	req->set_worker_id(2);
	req->set_input_port_mac("11:22:33:44:55:66");
	req->set_output_port_mac("22:33:44:55:66:77");
	req->set_control_port_mac("33:44:55:66:77:88");
	req->set_rpc_ip("192.168.1.1/30");
	req->set_rpc_port(80);
	*/

	//try, expect succeed result
	reply = nfa_rpc.AddReplicas(replicalist_request);

	if(reply){
		std::cout << "AddReplicas: OK "<< std::endl;
	}else{
		std::cout << "AddReplicas: Fail "<< std::endl;
	}
	//try again, expect fail result
	reply = nfa_rpc.AddReplicas(replicalist_request);

	if(reply){
		std::cout << "AddReplicas: OK "<< std::endl;
	}else{
		std::cout << "AddReplicas: Fail "<< std::endl;
	}

	return 0;
}
