#include "server_impl.h"

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

void ServerImpl::HandleRpcs(set<int> cpu_set, int lcore_id){
  cpu_set_t set;
  CPU_ZERO(&set);
  for(auto cpu_id : cpu_set){
    CPU_SET(cpu_id, &set);
  }
  rte_thread_set_affinity(&set);
  RTE_PER_LCORE(_lcore_id) = 0;

  STORE_BARRIER();

  create_call_data();



}

void ServerImpl::create_call_data(){

}
