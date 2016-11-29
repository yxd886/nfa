//
// This is where we implement all the RPC calls.

// This grpc server implementation should use the async modle, according
// to the implementation in this grpc greeter_async_server.cc example.

// This is because we must provide our own thread implementation and bond the
// thread for handling RPC calls to a DPDK EAL thread. So that this thread
// can use the DPDK ring.
#ifndef nfa_rpc_server
#define nfa_rpc_server


#define  NUL 0
#define  SAYHELLO 1
#define SAYHELLOAGAIN 2
#define LIVENESSCHECK 3


#endif
