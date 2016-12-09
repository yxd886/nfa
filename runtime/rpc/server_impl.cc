#include "server_impl.h"
#include "call_data_impl.h"

#include <glog/logging.h>

bool ServerImpl::Run(string rpc_ip, uint16_t rpc_port){
  string server_address = rpc_ip + string(":") + std::to_string(rpc_port);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service_);

  cq_ = builder.AddCompletionQueue();
  server_ = builder.BuildAndStart();

  if(server_==nullptr){
    LOG(ERROR)<<"RPC server fails to listen on address "<<server_address;
    return false;
  }
  else{
    LOG(INFO)<<"RPC server listsens on address "<<server_address;
    return true;
  }
}

void ServerImpl::HandleRpcs(set<int> cpu_set, int lcore_id, std::atomic<bool>& rpc_server_thread_ready){
  cpu_set_t set;
  CPU_ZERO(&set);
  for(auto cpu_id : cpu_set){
    CPU_SET(cpu_id, &set);
  }
  rte_thread_set_affinity(&set);
  RTE_PER_LCORE(_lcore_id) = lcore_id;

  create_call_data();
  void* tag;
  bool ok;

  rpc_server_thread_ready.store(true);
  while(true){
    GPR_ASSERT(cq_->Next(&tag, &ok));
    GPR_ASSERT(ok);
    static_cast<call_data_base*>(tag)->Proceed();
  }
}

void ServerImpl::create_call_data(){
  // new LivenessCheck(&service_, cq_.get());
  new derived_call_data<LivenessRequest, LivenessReply>(&service_, cq_.get());
  new derived_call_data<AddOutputRtsReq, AddOutputRtsRes>(&service_, cq_.get());
}
