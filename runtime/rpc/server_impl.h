#ifndef SERVER_IMPL_H
#define SERVER_IMPL_H

#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <set>

#include <grpc++/grpc++.h>
#include <grpc/support/log.h>

#include <rte_config.h>
#include <rte_lcore.h>
#include <rte_malloc.h>

#include "../bessport/kmod/llring.h"
#include "../bessport/nfa_msg.grpc.pb.h"
#include "../nfaflags.h"
#include "../bessport/utils/common.h"

using std::string;
using std::set;

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using nfa_msg::Runtime_RPC;

class ServerImpl final {
 public:
  ServerImpl(struct llring* rpc2worker_ring,
             struct llring* worker2rpc_ring) :
               rpc2worker_ring_(rpc2worker_ring),
               worker2rpc_ring_(worker2rpc_ring){}

  ~ServerImpl() {
    if(server_!=nullptr){
      server_->Shutdown();
    }
    if(cq_!=nullptr){
      cq_->Shutdown();
    }
  }

  bool Run(string rpc_ip, uint16_t rpc_port);

  void HandleRpcs(set<int> cpu_set, int lcore_id);

 private:

  void create_call_data();

  std::unique_ptr<ServerCompletionQueue> cq_;

  Runtime_RPC::AsyncService service_;

  std::unique_ptr<Server> server_;

  struct llring* rpc2worker_ring_;

  struct llring* worker2rpc_ring_;
};

#endif
