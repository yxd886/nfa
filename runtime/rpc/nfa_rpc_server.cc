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

#include "nfa_rpc_server.h"


ServerImpl::ServerImpl(int worker_id,struct rte_ring* rte_ring_request,struct rte_ring* rte_ring_reply)
	:worker_id(worker_id),rte_ring_request(rte_ring_request),rte_ring_reply(rte_ring_reply){

}
ServerImpl::~ServerImpl() {
	server_->Shutdown();
	// Always shutdown the completion queue after the server.
	cq_->Shutdown();
}

void ServerImpl::Run(int core) {
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

	// bind the server to specified core

	cpu_set_t set;

	CPU_ZERO(&set);
	CPU_SET(core, &set);
	rte_thread_set_affinity(&set);
	HandleRpcs();
}


void ServerImpl::HandleRpcs() {
	// Spawn rpc instances to serve new clients.

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


void runtime_thread(struct rte_ring* rte_ring_request,struct rte_ring* rte_ring_reply){
	std::cout<<"runtime thread ok"<<std::endl;

	void *request_ptr;
	struct reply_msg reply;
	RuntimeInfo runtimeinfo;
	RuntimeStat runtimestat;
	reply.runtime_info_msg_=&runtimeinfo;
	reply.runtime_stat_msg_=&runtimestat;
	int ok;
	while(1){

		sleep(2);
		ok=rte_ring_dequeue(rte_ring_request,&request_ptr);
		struct request_msg* request=static_cast<struct request_msg*>(request_ptr);
		if(ok==0){
			switch(request->action){
				case ADDOUTPUTVIEW:
					//process of addoutputview
					reply.worker_id=request->change_view_msg_.worker_id;
					break;
				case ADDINPUTVIEW:
					//process of addinputview
					reply.worker_id=request->change_view_msg_.worker_id;
					break;
				case DELETEOUTPUTVIEW:
					//process of deleteoutputview
					reply.worker_id=request->change_view_msg_.worker_id;
					break;
				case DELETEINPUTVIEW:
					//process of deleteinputview
					reply.worker_id=request->change_view_msg_.worker_id;
					break;
				case SETMIGRATIONTARGET:
					//process of setmigrationtarget
					reply.worker_id=request->change_migration_msg_.migration_target_info.worker_id;
					break;
				case ADDREPLICAS:
					//process of add replicas
					reply.worker_id=request->change_replica_msg_.replica.worker_id;
					std::cout<<"add replica worker_id: "<<request->change_replica_msg_.replica.worker_id<<std::endl;
					break;
				case DELETEREPLICAS:
					//process of delete replicas
					reply.worker_id=request->change_replica_msg_.replica.worker_id;
					std::cout<<"delete replica worker_id: "<<request->change_replica_msg_.replica.worker_id<<std::endl;
					break;
				case RECOVER:
					//process of recover
					reply.worker_id=request->set_recover_msg_.runtime_id;
					std::cout<<"recover runtime worker_id: "<<request->set_recover_msg_.runtime_id<<std::endl;
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

			reply.tag=request->action;

			reply.reply=true;
			std::cout<<"find request"<<std::endl;
			rte_ring_enqueue(rte_ring_reply,&reply);
		}else{
			std::cout<<"empty request queue"<<std::endl;
		}


	}
}


void rpc_server_thread(ServerImpl* server,struct rte_ring* rte_ring_request,struct rte_ring* rte_ring_reply){
	server->Run(2);

}



int main(int argc, char **argv) {

	int ret;
	static struct rte_mempool *mbuf_pool;

	//eal init
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		return -1;



	//init mempool
	mbuf_pool = rte_pktmbuf_pool_create("mbuf_pool", MBUF_PER_POOL,
			MBUF_POOL_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
			rte_socket_id());
	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "%s\n", rte_strerror(rte_errno));



  //init request and reply ring
	struct rte_ring * rte_ring_request,*rte_ring_reply;
	rte_ring_request = rte_ring_create("rte_ring_request", 4096, SOCKET_ID_ANY, RING_F_SP_ENQ | RING_F_SC_DEQ);
	if (NULL == rte_ring_request){
		std::cout<<"Rte ring create fail"<<std::endl;
		return -1;
	}
	std::cout<<"Rte ring create succeed"<<std::endl;
	rte_ring_reply = rte_ring_create("rte_ring_reply", 4096, SOCKET_ID_ANY, RING_F_SP_ENQ | RING_F_SC_DEQ);
	if (NULL == rte_ring_reply){
		std::cout<<"Rte ring create fail"<<std::endl;
		return -1;
	}
	//	struct rte_ring rte_ring_request;
	//	struct rte_ring rte_ring_reply;
	ServerImpl server(1,rte_ring_request,rte_ring_reply);
	std::cout<<"begin to create threads"<<std::endl;
	std::thread t1(runtime_thread,rte_ring_request,rte_ring_reply);//create runtime thread
	std::thread t2(rpc_server_thread,&server,rte_ring_request,rte_ring_reply);//create rpc_server thread

	while(1){
		sleep(1);

	}//main thread loop forever


	return 0;
}



