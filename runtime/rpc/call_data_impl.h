#ifndef CALL_DATA_IMPL
#define CALL_DATA_IMPL

#include <unordered_map>
#include <memory>
#include <thread>
#include <chrono>
#include <string>

#include "call_data_base.h"
#include "../bessport/kmod/llring.h"
#include "../nfaflags.h"
#include "ring_msg.h"

using std::string;
using std::unordered_map;

using namespace nfa_msg;

template<class TReq, class TRep>
class derived_call_data : public call_data_base{
public:
  derived_call_data(Runtime_RPC::AsyncService* service,
                    ServerCompletionQueue* cq,
                    struct llring* rpc2worker_ring,
                    struct llring* worker2rpc_ring,
                    unordered_map<string, runtime_config>& input_runtimes,
                    unordered_map<string, runtime_config>& output_runtimes,
                    unordered_map<string, runtime_config>& replicas,
                    unordered_map<string, runtime_config>& storages,
                    runtime_config& migration_target,
                    runtime_config& local_runtime)
    : call_data_base(service, cq),
      responder_(&ctx_),
      rpc2worker_ring_(rpc2worker_ring),
      worker2rpc_ring_(worker2rpc_ring),
      input_runtimes_(input_runtimes),
      output_runtimes_(output_runtimes),
      replicas_(replicas),
      storages_(storages),
      migration_target_(migration_target),
      local_runtime_(local_runtime){
    Proceed();
  }

  void Proceed() override{}

  ~derived_call_data() = default;

private:
  void create_itself(){
    new derived_call_data<TReq, TRep>(service_,
                                      cq_,
                                      rpc2worker_ring_,
                                      worker2rpc_ring_,
                                      input_runtimes_,
                                      output_runtimes_,
                                      replicas_,
                                      storages_,
                                      migration_target_,
                                      local_runtime_);
  }

  void* poll_worker2rpc_ring(){
    int aggressive_poll_attemps = 50;
    int flag = 0;
    void* dequeue_output[1];

    for(int i=0; i<aggressive_poll_attemps; i++){
      flag = llring_sc_dequeue(worker2rpc_ring_, dequeue_output);

      if(flag != 0){
        continue;
      }
      else{
        return dequeue_output[0];
      }
    }

    for(;;){
      flag = llring_sc_dequeue(worker2rpc_ring_, dequeue_output);

      if(flag != 0){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      else{
        return dequeue_output[0];
      }
    }
  }

  TReq request_;

  TRep reply_;

  ServerAsyncResponseWriter<TRep> responder_;

  struct llring* rpc2worker_ring_;

  struct llring* worker2rpc_ring_;

  unordered_map<string, runtime_config>& input_runtimes_;

  unordered_map<string, runtime_config>& output_runtimes_;

  unordered_map<string, runtime_config>& replicas_;

  unordered_map<string, runtime_config>& storages_;

  runtime_config& migration_target_;

  runtime_config& local_runtime_;

  inline string concat_with_colon(const string& s1, const string&s2){
    return s1+string(":")+s2;
  }
};

// The following code is the template for implementing the RPC call.
// template<>
// void derived_call_data<RPCRequestType, RPCResponseType>::Proceed(){
//   if (status_ == CREATE) {
//     status_ = PROCESS;
//     service_->RequestRPCCallName(&ctx_, &request_, &responder_, cq_, cq_, this);
//   } else if (status_ == PROCESS) {
//     create_itself();
//
    // Where the actual handling is done.

//     status_ = FINISH;
//     responder_.Finish(reply_, Status::OK, this);
//   } else {
//     GPR_ASSERT(status_ == FINISH);
//     delete this;
//   }
// }


// RPC implementation for LivenessCheck

template<>
void derived_call_data<LivenessRequest, LivenessReply>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestLivenessCheck(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for AddOutputRts

template<>
void derived_call_data<AddOutputRtsReq, AddOutputRtsRes>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestAddOutputRts(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    RuntimeConfig protobuf_local_runtime =  local2protobuf(local_runtime_);

    for(int i=0; i<request_.addrs_size(); i++){
      string dest_addr = concat_with_colon(request_.addrs(i).rpc_ip(),
                                           std::to_string(request_.addrs(i).rpc_port()));
      if(output_runtimes_.find(dest_addr)!=output_runtimes_.end()){
        continue;
      }

      std::unique_ptr<Runtime_RPC::Stub> stub(Runtime_RPC::NewStub(
          grpc::CreateChannel(dest_addr, grpc::InsecureChannelCredentials())));

      AddInputRtReq req;
      req.mutable_input_runtime()->CopyFrom(protobuf_local_runtime);
      AddInputRtRep rep;

      ClientContext context;
      std::chrono::system_clock::time_point deadline =
              std::chrono::system_clock::now() + std::chrono::seconds(FLAGS_rpc_timeout);
      context.set_deadline(deadline);

      Status status = stub->AddInputRt(&context, req, &rep);

      if(status.ok() && rep.has_local_runtime()){
        runtime_config output_runtime = protobuf2local(rep.local_runtime());

        llring_item item(rpc_operation::add_output_runtime, output_runtime, 0, 0);

        llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

        poll_worker2rpc_ring();
      }
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for AddInputRt

template<>
void derived_call_data<AddInputRtReq, AddInputRtRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestAddInputRt(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    runtime_config input_runtime = protobuf2local(request_.input_runtime());
    string input_runtime_addr = concat_with_colon(request_.input_runtime().rpc_ip(),
                                                  std::to_string(request_.input_runtime().rpc_port()));
    if((input_runtime != local_runtime_)&&
        (input_runtimes_.find(input_runtime_addr)==input_runtimes_.end())){
      input_runtimes_.emplace(input_runtime_addr, input_runtime);

      llring_item item(rpc_operation::add_input_runtime, input_runtime, 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();

      RuntimeConfig protobuf_local_runtime =  local2protobuf(local_runtime_);

      reply_.mutable_local_runtime()->CopyFrom(protobuf_local_runtime);
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}


// RPC implementation for DeleteOutputRt

template<>
void derived_call_data<DeleteOutputRtReq, DeleteOutputRtRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteOutputRt(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    string output_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                   std::to_string(request_.addrs().rpc_port()));
    if((output_runtimes_.find(output_runtime_addr)!=output_runtimes_.end())){

      llring_item item(rpc_operation::delete_output_runtime, output_runtimes_[output_runtime_addr], 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();

      output_runtimes_.erase(output_runtime_addr);
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}


// RPC implementation for DeleteInputRt

template<>
void derived_call_data<DeleteInputRtReq, DeleteInputRtRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestDeleteInputRt(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();

    string input_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                   std::to_string(request_.addrs().rpc_port()));
    if((input_runtimes_.find(input_runtime_addr)!=input_runtimes_.end())){

      llring_item item(rpc_operation::delete_input_runtime, input_runtimes_[input_runtime_addr], 0, 0);

      llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

      poll_worker2rpc_ring();
      
      input_runtimes_.erase(input_runtime_addr);
    }

    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

//RPC implementation for SetMigrationTarget
template<>
void derived_call_data<SetMigrationTargetReq, SetMigrationTargetRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestSetMigrationTarget(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    reply_.set_succeed(false);

    RuntimeConfig protobuf_local_runtime =  local2protobuf(local_runtime_);

		string dest_addr = concat_with_colon(request_.addr().rpc_ip(),
																				 std::to_string(request_.addr().rpc_port()));
		string local_addr = concat_with_colon(convert_uint32t_ip(local_runtime_.rpc_ip),
																				 std::to_string(local_runtime_.rpc_port));
		if(dest_addr==local_addr){
	    status_ = FINISH;
	    responder_.Finish(reply_, Status::OK, this);
	    return;
		}

		std::unique_ptr<Runtime_RPC::Stub> stub(Runtime_RPC::NewStub(
				grpc::CreateChannel(dest_addr, grpc::InsecureChannelCredentials())));

		MigrationNegotiateReq req;
		req.set_quota(request_.quota());
		for(unordered_map<string, runtime_config>::iterator it=input_runtimes_.begin();it!=input_runtimes_.end();it++){
			req.add_input_runtime_addrs()->set_rpc_ip(convert_uint32t_ip(it->rpc_ip));
			req.add_input_runtime_addrs()->set_rpc_port(it->rpc_port);

		}
		for(unordered_map<string, runtime_config>::iterator it=output_runtimes_.begin();it!=output_runtimes_.end();it++){
			req.add_output_runtime_addrs()->set_rpc_ip(convert_uint32t_ip(it->rpc_ip));
			req.add_output_runtime_addrs()->set_rpc_port(it->rpc_port);

		}
		MigrationNegotiateRep rep;

		ClientContext context;
		std::chrono::system_clock::time_point deadline =
						std::chrono::system_clock::now() + std::chrono::seconds(FLAGS_rpc_timeout);
		context.set_deadline(deadline);

		Status status = stub->MigrationNegotiate(&context, req, &rep);

		if(status.ok()){
	    reply_.set_succeed(true);

			llring_item item(rpc_operation::set_migration_target, 0, rep.quota(), 0);

			llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

			poll_worker2rpc_ring();
		}


    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

// RPC implementation for MigrationNegotiation

template<>
void derived_call_data<MigrationNegotiateReq, MigrationNegotiateRep>::Proceed(){
  if (status_ == CREATE) {
    status_ = PROCESS;
    service_->RequestMigrationNegotiate(&ctx_, &request_, &responder_, cq_, cq_, this);
  } else if (status_ == PROCESS) {
    create_itself();
    reply_.set_succeed(false);

    string input_runtime_addr = concat_with_colon(request_.addrs().rpc_ip(),
                                                   std::to_string(request_.addrs().rpc_port()));
    if(input_runtimes_.size()!=request_.input_runtime_addrs_size()||output_runtimes_.size()!=request_.output_runtime_addrs_size()){

    	status_ = FINISH;
      responder_.Finish(reply_, Status::OK, this);
      return;
    }

    for(int i=0;i<request_.input_runtime_addrs_size();i++){
      string compare_addr = concat_with_colon(request_.input_runtime_addrs(i).rpc_ip(),
                                                     std::to_string(request_.input_runtime_addrs(i).rpc_port()));
    	if(input_runtimes_.find(compare_addr)==input_runtime_.end()){
      	status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
        return;
    	}
    }
    for(int i=0;i<request_.output_runtime_addrs_size();i++){
      string compare_addr = concat_with_colon(request_.output_runtime_addrs(i).rpc_ip(),
                                                     std::to_string(request_.output_runtime_addrs(i).rpc_port()));
    	if(output_runtimes_.find(compare_addr)==output_runtime_.end()){
      	status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
        return;
    	}
    }

		llring_item item(rpc_operation::migration_negotiate, 0, request_.quota(), 0);

		llring_sp_enqueue(rpc2worker_ring_, static_cast<void*>(&item));

		poll_worker2rpc_ring();

    reply_.set_succeed(true);
    reply_.set_quota(item.migration_qouta);
    status_ = FINISH;
    responder_.Finish(reply_, Status::OK, this);
  } else {
    GPR_ASSERT(status_ == FINISH);
    delete this;
  }
}

#endif
