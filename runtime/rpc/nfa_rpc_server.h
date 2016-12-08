// This is where we implement all the RPC calls.

// This grpc server implementation should use the async modle, according
// to the implementation in this grpc greeter_async_server.cc example.

// This is because we must provide our own thread implementation and bond the
// thread for handling RPC calls to a DPDK EAL thread. So that this thread
// can use the DPDK ring.

//
// This is where we implement all the RPC calls.

// This grpc server implementation should use the async modle, according
// to the implementation in this grpc greeter_async_server.cc example.

// This is because we must provide our own thread implementation and bond the
// thread for handling RPC calls to a DPDK EAL thread. So that this thread
// can use the DPDK ring.
#ifndef NFA_rpc_server
#define NFA_rpc_server


#include <memory>
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <map>
#include <queue>
#include <unistd.h>
#include <sys/shm.h>
#include <rte_common.h>
#include <rte_errno.h>
#include <rte_config.h>
#include <rte_ring.h>
#include <rte_malloc.h>
#include <rte_eal.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_lcore.h>

#include <grpc++/grpc++.h>

#include "../bessport/nfa_msg.grpc.pb.h"

#include "add_inputview.h"
#include "add_outputview.h"
#include "add_replicas.h"
#include "delete_inputview.h"
#include "delete_outputview.h"
#include "delete_replicas.h"
#include "liveness_check.h"
#include "query_runtimeinfo.h"
#include "query_runtimestat.h"
#include "recover.h"
#include "set_migration_target.h"

#define MBUF_PER_POOL 65535
#define MBUF_POOL_CACHE_SIZE 250

#include "rpc_common.h"

class ServerImpl final {
public:
	ServerImpl(int worker_id,struct rte_ring* rte_ring_request,struct rte_ring* rte_ring_reply);

	~ServerImpl();

	void Run(int core);

private:

	// This can be run in multiple threads if needed.
	void HandleRpcs();

	std::unique_ptr<ServerCompletionQueue> cq_;
	Runtime_RPC::AsyncService service_;
	std::unique_ptr<Server> server_;
	std::map<int , struct Local_view> viewlist_input;
	std::map< int, struct Local_view> viewlist_output;
	std::map< int, struct Local_view>  replicalist;
public:
	struct rte_ring* rte_ring_request;
	struct rte_ring* rte_ring_reply;
	int worker_id;
};



#endif
