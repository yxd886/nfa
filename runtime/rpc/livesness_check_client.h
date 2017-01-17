#include <iostream>
#include <memory>
#include <string>
#include <map>

#include <grpc++/grpc++.h>
#include <glog/logging.h>

#include "../bessport/nfa_msg.grpc.pb.h"
#include "ring_msg.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using nfa_msg::LivenessRequest;
using nfa_msg::LivenessReply;
using nfa_msg::Runtime_RPC;

using namespace nfa_msg;

std::string concat_with_colon(const std::string& s1, const std::string&s2){
  return s1+std::string(":")+s2;
}


struct portstate{
  uint64_t input_port_incoming_pkts;
  uint64_t input_port_outgoing_pkts;
  uint64_t input_port_dropped_pkts;
  uint64_t output_port_incoming_pkts;
  uint64_t output_port_outgoing_pkts;
  uint64_t output_port_dropped_pkts;
  uint64_t control_port_incoming_pkts;
  uint64_t control_port_outgoing_pkts;
  uint64_t control_port_dropped_pkts;
};

struct flowstate{
	uint64_t active_flows;
	uint64_t inactive_flows;
};

struct migrationstate{
	uint64_t migration_index;
	uint64_t migration_target_runtime_id;
  uint64_t migration_qouta;
  uint64_t average_flow_migration_completion_time;
  uint64_t toal_flow_migration_completion_time;
  uint64_t successful_migration;
};

struct storagestate{
	uint64_t replication_source_runtime_id;
	uint64_t num_of_flow_replicas;
	uint64_t total_replay_time;
};


struct runtime_state{
	portstate port_state;
	flowstate flow_state;
	migrationstate migration_state;
	storagestate storage_states;
	std::map<std::string,runtime_config>input_runtimes;
	std::map<std::string,runtime_config> output_runtimes;
	std::map<std::string,runtime_config> replicas;
	std::map<std::string,runtime_config> storages;
	runtime_config migration_target;
	runtime_config local_runtime;
};



class LivenessCheckClient {
 public:
  LivenessCheckClient(std::shared_ptr<Channel> channel)
      : stub_(Runtime_RPC::NewStub(channel)) {}

  std::string Check() {

    LivenessRequest request;

    LivenessReply reply;

    ClientContext context;

    // The actual RPC.
    Status status = stub_->LivenessCheck(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return "LivenessCheck succeed";
    } else {
      return "LivenessCheck fail";
    }
  }

  std::string AddOutputRt(){
    AddOutputRtsReq request;
    AddOutputRtsRes reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip("202.45.128.155");
    new_addr_ptr->set_rpc_port(10241);

    new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip("202.45.128.156");
    new_addr_ptr->set_rpc_port(10242);

    Status status = stub_->AddOutputRts(&context, request, &reply);

    if(status.ok()){
      return "AddOutputRt finishes.";
    }
    else{
      return "AddOutputRt fails.";
    }
  }

  std::string SingleAddOutputRt(std::string ip,int32_t port_num){
    AddOutputRtsReq request;
    AddOutputRtsRes reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip(ip);
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->AddOutputRts(&context, request, &reply);

    if(status.ok()){
      return "AddOutputRt finishes.";
    }
    else{
      return "AddOutputRt fails.";
    }
  }

  std::string DeleteOutputRt(std::string ip,int32_t port_num){
    DeleteOutputRtReq request;
    DeleteOutputRtRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteOutputRt(&context, request, &reply);

    if(status.ok()){
      return "DeleteOutputRt finishes.";
    }
    else{
      return "DeleteOutputRt fails.";
    }
  }

  std::string DeleteInputRt(std::string ip,int32_t port_num){
    DeleteInputRtReq request;
    DeleteInputRtRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteInputRt(&context, request, &reply);

    if(status.ok()){
      return "DeleteInputRt finishes.";
    }
    else{
      return "DeleteInputRt fails.";
    }
  }

  std::string AddOutputMac(std::string ip,int32_t port_num){
    AddOutputMacReq request;
    AddOutputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->AddOutputMac(&context, request, &reply);

    if(status.ok()){
      return "AddOutputMac finishes.";
    }
    else{
      return "AddOutputMac fails.";
    }
  }

  std::string AddInputMac(std::string ip,int32_t port_num){
    AddInputMacReq request;
    AddInputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->AddInputMac(&context, request, &reply);

    if(status.ok()){
      return "AddInputMac finishes.";
    }
    else{
      return "AddInputMac fails.";
    }
  }

  std::string DeleteOutputMac(std::string ip,int32_t port_num){
    DeleteOutputMacReq request;
    DeleteOutputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteOutputMac(&context, request, &reply);

    if(status.ok()){
      return "DeleteOutputMac finishes.";
    }
    else{
      return "DeleteOutputMac fails.";
    }
  }

  std::string DeleteInputMac(std::string ip,int32_t port_num){
    DeleteInputMacReq request;
    DeleteInputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteInputMac(&context, request, &reply);

    if(status.ok()){
      return "DeleteInputMac finishes.";
    }
    else{
      return "DeleteInputMac fails.";
    }
  }

  std::string MigrateTo(std::string ip,int32_t port_num, int32_t quota){
    MigrateToReq request;
    MigrateToRep reply;
    ClientContext context;

    request.mutable_addr()->set_rpc_ip(ip);
    request.mutable_addr()->set_rpc_port(port_num);
    request.set_quota(quota);

    Status status = stub_->MigrateTo(&context, request, &reply);

    if(status.ok()){
      return "MigrateTo finishes.";
    }
    else{
      return "MigrateTo fails.";
    }
  }

  std::string SetMigrationTarget(std::string ip,int32_t port_num, int32_t qouta){
    SetMigrationTargetReq request;
    SetMigrationTargetRep reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip(ip);
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->SetMigrationTarget(&context, request, &reply);

    if(status.ok()){
      return "SetMigrationTarget finishes.";
    }
    else{
      return "SetMigrationTarget fails.";
    }
  }

  std::string AddReplicas(std::string ip,int32_t port_num){
    AddReplicasReq request;
    AddReplicasRep reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip(ip);
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->AddReplicas(&context, request, &reply);

    if(status.ok()){
      return "AddReplicas finishes.";
    }
    else{
      return "AddReplicas fails.";
    }
  }

  std::string DeleteReplica(std::string ip,int32_t port_num){
    DeleteReplicaReq request;
    DeleteReplicaRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteReplica(&context, request, &reply);

    if(status.ok()){
      return "DeleteReplica finishes.";
    }
    else{
      return "DeleteReplica fails.";
    }
  }

  std::string DeleteStorage(std::string ip,int32_t port_num){
    DeleteStorageReq request;
    DeleteStorageRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip(ip);
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteStorage(&context, request, &reply);

    if(status.ok()){
      return "DeleteStorage finishes.";
    }
    else{
      return "DeleteStorage fails.";
    }
  }

  std::string Recover(std::string ip,int32_t port_num){
    RecoverReq request;
    RecoverRep reply;
    ClientContext context;

    request.mutable_addr()->set_rpc_ip(ip);
    request.mutable_addr()->set_rpc_port(port_num);

    Status status = stub_->Recover(&context, request, &reply);

    if(status.ok()){
      return "Recover finishes.";
    }
    else{
      return "Recover fails.";
    }
  }

  std::string GetRuntimeState(runtime_state& runtime_stat){
    GetRuntimeStateReq request;
    GetRuntimeStateRep reply;
    ClientContext context;

    Status status = stub_->GetRuntimeState(&context, request, &reply);

    LOG(INFO)<<"The rpc ip of the runtime is "<<reply.local_runtime().rpc_ip();
    LOG(INFO)<<"The rpc port of the runtime is "<<reply.local_runtime().rpc_port();

    copy_runtime_stat_to_local(runtime_stat,reply);

    if(status.ok()){
      return "GetRuntimeStateReq finishes.";
    }
    else{
      return "GetRuntimeStateReq fails.";
    }
  }


  std::string ShutdownRuntime(){
  	ShutdownRuntimeReq request;
  	ShutdownRuntimeRep reply;
    ClientContext context;

    Status status = stub_->ShutdownRuntime(&context, request, &reply);

    if(status.ok()){
      return "ShutdownRuntime finishes.";
    }
    else{
      return "ShutdownRuntime fails.";
    }
  }

 private:
  std::unique_ptr<Runtime_RPC::Stub> stub_;
  void copy_runtime_stat_to_local(runtime_state& runtime_stat,GetRuntimeStateRep reply){
  	runtime_stat.port_state.control_port_dropped_pkts=reply.port_state().control_port_dropped_pkts();
  	runtime_stat.port_state.control_port_incoming_pkts=reply.port_state().control_port_incoming_pkts();
  	runtime_stat.port_state.control_port_outgoing_pkts=reply.port_state().control_port_outgoing_pkts();
  	runtime_stat.port_state.input_port_dropped_pkts=reply.port_state().input_port_dropped_pkts();
		runtime_stat.port_state.input_port_incoming_pkts=reply.port_state().input_port_incoming_pkts();
		runtime_stat.port_state.input_port_outgoing_pkts=reply.port_state().input_port_outgoing_pkts();
		runtime_stat.port_state.output_port_dropped_pkts=reply.port_state().output_port_dropped_pkts();
		runtime_stat.port_state.output_port_incoming_pkts=reply.port_state().output_port_incoming_pkts();
		runtime_stat.port_state.output_port_outgoing_pkts=reply.port_state().output_port_outgoing_pkts();
		runtime_stat.flow_state.active_flows=reply.flow_state().active_flows();
		runtime_stat.flow_state.inactive_flows=reply.flow_state().inactive_flows();
		runtime_stat.migration_state.average_flow_migration_completion_time=reply.migration_state().average_flow_migration_completion_time();
		runtime_stat.migration_state.migration_index=reply.migration_state().migration_index();
		runtime_stat.migration_state.migration_qouta=reply.migration_state().migration_qouta();
		runtime_stat.migration_state.migration_target_runtime_id=reply.migration_state().migration_target_runtime_id();
		runtime_stat.migration_state.successful_migration=reply.migration_state().successful_migration();
		runtime_stat.migration_state.toal_flow_migration_completion_time=reply.migration_state().toal_flow_migration_completion_time();
		runtime_stat.storage_states.num_of_flow_replicas=reply.storage_states().num_of_flow_replicas();
		runtime_stat.storage_states.replication_source_runtime_id=reply.storage_states().replication_source_runtime_id();
		runtime_stat.storage_states.total_replay_time=reply.storage_states().total_replay_time();
		runtime_stat.migration_target=protobuf2local(reply.migration_target());
		runtime_stat.local_runtime=protobuf2local(reply.local_runtime());
		for(int i =0; i<reply.input_runtimes_size();i++){

      std::string dest_addr = concat_with_colon(reply.input_runtimes(i).rpc_ip(),
                                           std::to_string(reply.input_runtimes(i).rpc_port()));
      runtime_config input_runtime = protobuf2local(reply.input_runtimes(i));
      runtime_stat.input_runtimes.emplace(dest_addr,input_runtime);

		}

		for(int i =0; i<reply.output_runtimes_size();i++){

      std::string dest_addr = concat_with_colon(reply.output_runtimes(i).rpc_ip(),
                                           std::to_string(reply.output_runtimes(i).rpc_port()));
      runtime_config output_runtime = protobuf2local(reply.output_runtimes(i));
      runtime_stat.output_runtimes.emplace(dest_addr,output_runtime);

		}

		for(int i =0; i<reply.replicas_size();i++){

      std::string dest_addr = concat_with_colon(reply.replicas(i).rpc_ip(),
                                           std::to_string(reply.replicas(i).rpc_port()));
      runtime_config replica = protobuf2local(reply.replicas(i));
      runtime_stat.replicas.emplace(dest_addr,replica);

		}


		for(int i =0; i<reply.storages_size();i++){

      std::string dest_addr = concat_with_colon(reply.storages(i).rpc_ip(),
                                           std::to_string(reply.storages(i).rpc_port()));
      runtime_config storage = protobuf2local(reply.storages(i));
      runtime_stat.storages.emplace(dest_addr,storage);

		}

  }

};
