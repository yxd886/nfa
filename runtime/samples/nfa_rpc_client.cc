
#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include <glog/logging.h>

#include "../bessport/nfa_msg.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using nfa_msg::LivenessRequest;
using nfa_msg::LivenessReply;
using nfa_msg::Runtime_RPC;

using namespace nfa_msg;

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
    new_addr_ptr->set_rpc_ip("127.0.0.1");
    new_addr_ptr->set_rpc_port(10241);

    new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip("127.0.0.1");
    new_addr_ptr->set_rpc_port(10242);

    Status status = stub_->AddOutputRts(&context, request, &reply);

    if(status.ok()){
      return "AddOutputRt finishes.";
    }
    else{
      return "AddOutputRt fails.";
    }
  }

  std::string SingleAddOutputRt(int32_t port_num){
    AddOutputRtsReq request;
    AddOutputRtsRes reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip("127.0.0.1");
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->AddOutputRts(&context, request, &reply);

    if(status.ok()){
      return "AddOutputRt finishes.";
    }
    else{
      return "AddOutputRt fails.";
    }
  }

  std::string DeleteOutputRt(int32_t port_num){
    DeleteOutputRtReq request;
    DeleteOutputRtRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip("127.0.0.1");
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteOutputRt(&context, request, &reply);

    if(status.ok()){
      return "DeleteOutputRt finishes.";
    }
    else{
      return "DeleteOutputRt fails.";
    }
  }

  std::string DeleteInputRt(int32_t port_num){
    DeleteInputRtReq request;
    DeleteInputRtRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip("127.0.0.1");
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteInputRt(&context, request, &reply);

    if(status.ok()){
      return "DeleteInputRt finishes.";
    }
    else{
      return "DeleteInputRt fails.";
    }
  }

  std::string AddOutputMac(int32_t port_num){
    AddOutputMacReq request;
    AddOutputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip("127.0.0.1");
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->AddOutputMac(&context, request, &reply);

    if(status.ok()){
      return "AddOutputMac finishes.";
    }
    else{
      return "AddOutputMac fails.";
    }
  }

  std::string AddInputMac(int32_t port_num){
    AddInputMacReq request;
    AddInputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip("127.0.0.1");
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->AddInputMac(&context, request, &reply);

    if(status.ok()){
      return "AddInputMac finishes.";
    }
    else{
      return "AddInputMac fails.";
    }
  }

  std::string DeleteOutputMac(int32_t port_num){
    DeleteOutputMacReq request;
    DeleteOutputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip("127.0.0.1");
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteOutputMac(&context, request, &reply);

    if(status.ok()){
      return "DeleteOutputMac finishes.";
    }
    else{
      return "DeleteOutputMac fails.";
    }
  }

  std::string DeleteInputMac(int32_t port_num){
    DeleteInputMacReq request;
    DeleteInputMacRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip("127.0.0.1");
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteInputMac(&context, request, &reply);

    if(status.ok()){
      return "DeleteInputMac finishes.";
    }
    else{
      return "DeleteInputMac fails.";
    }
  }

  std::string MigrateTo(int32_t port_num, int32_t quota){
    MigrateToReq request;
    MigrateToRep reply;
    ClientContext context;

    request.mutable_addr()->set_rpc_ip("127.0.0.1");
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

  std::string SetMigrationTarget(int32_t port_num, int32_t qouta){
    SetMigrationTargetReq request;
    SetMigrationTargetRep reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip("127.0.0.1");
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->SetMigrationTarget(&context, request, &reply);

    if(status.ok()){
      return "SetMigrationTarget finishes.";
    }
    else{
      return "SetMigrationTarget fails.";
    }
  }

  std::string AddReplicas(int32_t port_num){
    AddReplicasReq request;
    AddReplicasRep reply;
    ClientContext context;

    auto new_addr_ptr = request.add_addrs();
    new_addr_ptr->set_rpc_ip("127.0.0.1");
    new_addr_ptr->set_rpc_port(port_num);

    Status status = stub_->AddReplicas(&context, request, &reply);

    if(status.ok()){
      return "AddReplicas finishes.";
    }
    else{
      return "AddReplicas fails.";
    }
  }

  std::string DeleteReplica(int32_t port_num){
    DeleteReplicaReq request;
    DeleteReplicaRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip("127.0.0.1");
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteReplica(&context, request, &reply);

    if(status.ok()){
      return "DeleteReplica finishes.";
    }
    else{
      return "DeleteReplica fails.";
    }
  }

  std::string DeleteStorage(int32_t port_num){
    DeleteStorageReq request;
    DeleteStorageRep reply;
    ClientContext context;

    request.mutable_addrs()->set_rpc_ip("127.0.0.1");
    request.mutable_addrs()->set_rpc_port(port_num);

    Status status = stub_->DeleteStorage(&context, request, &reply);

    if(status.ok()){
      return "DeleteStorage finishes.";
    }
    else{
      return "DeleteStorage fails.";
    }
  }

  std::string GetRuntimeState(){
    GetRuntimeStateReq request;
    GetRuntimeStateRep reply;
    ClientContext context;

    Status status = stub_->GetRuntimeState(&context, request, &reply);

    LOG(INFO)<<"The rpc ip of the runtime is "<<reply.local_runtime().rpc_ip();
    LOG(INFO)<<"The rpc port of the runtime is "<<reply.local_runtime().rpc_port();

    if(status.ok()){
      return "GetRuntimeStateReq finishes.";
    }
    else{
      return "GetRuntimeStateReq fails.";
    }
  }

 private:
  std::unique_ptr<Runtime_RPC::Stub> stub_;
};

int main(int argc, char** argv) {
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  LivenessCheckClient checker_10240(grpc::CreateChannel(
      "localhost:10240", grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_10241(grpc::CreateChannel(
        "localhost:10241", grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_10242(grpc::CreateChannel(
        "localhost:10242", grpc::InsecureChannelCredentials()));

  // LOG(INFO)<<checker_10240.Check();
  // LOG(INFO)<<checker_10241.Check();
  // LOG(INFO)<<checker_10242.Check();

  // add itself
  // LOG(INFO)<<checker_10240.SingleAddOutputRt(10240);

  // add error address
  // LOG(INFO)<<checker_10240.SingleAddOutputRt(10340);

  // Test add/delete input/output runtimes.
  // LOG(INFO)<<checker_10240.AddOutputRt();

  // LOG(INFO)<<checker_10240.AddOutputMac(10241);
  // LOG(INFO)<<checker_10240.AddOutputMac(10242);
  // LOG(INFO)<<checker_10241.AddInputMac(10240);
  // LOG(INFO)<<checker_10242.AddInputMac(10240);

  // LOG(INFO)<<checker_10240.DeleteOutputMac(10241);
  // LOG(INFO)<<checker_10240.DeleteOutputMac(10242);
  // LOG(INFO)<<checker_10241.DeleteInputMac(10240);
  // LOG(INFO)<<checker_10242.DeleteInputMac(10240);

  //LOG(INFO)<<checker_10240.DeleteOutputRt(10241);
  //LOG(INFO)<<checker_10240.DeleteOutputRt(10242);
  //LOG(INFO)<<checker_10241.DeleteInputRt(10240);
  //LOG(INFO)<<checker_10242.DeleteInputRt(10240);

  // Test migration
  // LOG(INFO)<<checker_10240.AddOutputRt();
  // LOG(INFO)<<checker_10241.SetMigrationTarget(10242,1000);

  // Test migration between runtimes with different input/output runtimes
  // LOG(INFO)<<checker_10242.DeleteInputRt(10240);
  // LOG(INFO)<<checker_10241.SetMigrationTarget(10242,1000);

  // Recover
  // LOG(INFO)<<checker_10240.DeleteOutputRt(10241);
  // LOG(INFO)<<checker_10240.DeleteOutputRt(10242);
  // LOG(INFO)<<checker_10241.DeleteInputRt(10240);
  // LOG(INFO)<<checker_10242.DeleteInputRt(10240);

  // Test set replication
  // LOG(INFO)<<checker_10240.AddOutputRt();
  // LOG(INFO)<<checker_10240.AddOutputMac(10242);
  // LOG(INFO)<<checker_10240.AddOutputMac(10241);
  // LOG(INFO)<<checker_10241.SetMigrationTarget(10242,1000);

  // LOG(INFO)<<checker_10241.AddReplicas(10242);
  //LOG(INFO)<<checker_10242.AddReplicas(10241);
  //LOG(INFO)<<checker_10241.DeleteReplica(10242);
  //LOG(INFO)<<checker_10241.DeleteStorage(10242);
  //LOG(INFO)<<checker_10242.DeleteReplica(10241);
  //LOG(INFO)<<checker_10242.DeleteStorage(10241);

  // Test set replicas between runtimes with different input/output runtimes
  // LOG(INFO)<<checker_10240.AddOutputRt();
  // LOG(INFO)<<checker_10242.DeleteInputRt(10240);
  // LOG(INFO)<<checker_10241.AddReplicas(10242);

  // Test remove replica and storage
  // LOG(INFO)<<checker_10240.AddOutputRt();
  // LOG(INFO)<<checker_10241.AddReplicas(10242);
  // LOG(INFO)<<checker_10241.DeleteReplica(10242);
  // LOG(INFO)<<checker_10242.DeleteStorage(10241);

  // LOG(INFO)<<checker_10240.AddOutputRt();
  // LOG(INFO)<<checker_10240.GetRuntimeState();
  // LOG(INFO)<<checker_10241.GetRuntimeState();
  // LOG(INFO)<<checker_10242.GetRuntimeState();

  // Test mptcp application
     LOG(INFO)<<checker_10240.AddOutputRt();

  // LOG(INFO)<<checker_10240.AddOutputMac(10241);
     LOG(INFO)<<checker_10240.AddOutputMac(10242);
  // LOG(INFO)<<checker_10241.AddInputMac(10240);
     LOG(INFO)<<checker_10242.AddInputMac(10240);
     LOG(INFO)<<checker_10242.SetMigrationTarget(10241,1000);
     LOG(INFO)<<checker_10242.MigrateTo(10241,1000);

  return 0;
}
