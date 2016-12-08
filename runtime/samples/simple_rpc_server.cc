#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <rte_config.h>
#include <rte_lcore.h>
#include <rte_malloc.h>

#include <glog/logging.h>
#include <grpc++/grpc++.h>
#include <grpc/support/log.h>

#include "../nfaflags.h"
#include "../nfadpdk.h"
#include "../bessport/helloworld.grpc.pb.h"
#include "../bessport/utils/common.h"
#include "../bessport/kmod/llring.h"

using namespace std;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

DEFINE_bool(test_rpc, true, "The program acts as an RPC server if this flag is set to true. Otherwise test llring");
DEFINE_bool(recursive_call, false, "The program recursively calls 0.0.0.0:50052");

static volatile bool server_running = false;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

class ServerImpl final {
 public:
  ServerImpl(struct llring* ring) :
    ring_(ring){}

  ~ServerImpl() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run() {
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

    // Proceed to the server's main loop.
    HandleRpcs();
  }

 private:
  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(Greeter::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      // Invoke the serving logic right away.
      Proceed();
    }

    void Proceed() {
      if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);

        if(FLAGS_recursive_call==true){
          GreeterClient greeter(grpc::CreateChannel(
                "localhost:50052", grpc::InsecureChannelCredentials()));
          std::string user("world");
          std::string reply = greeter.SayHello(user);
          std::cout << "Greeter received: " << reply << std::endl;
        }

        // The actual processing.
        std::string prefix("Hello ");
        reply_.set_message(prefix + request_.name());

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
      } else {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

   private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    Greeter::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue* cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;

    // What we get from the client.
    HelloRequest request_;
    // What we send back to the client.
    HelloReply reply_;

    // The means to get back to the client.
    ServerAsyncResponseWriter<HelloReply> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };

  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(1, &set);
    rte_thread_set_affinity(&set);

    RTE_PER_LCORE(_lcore_id) = 0;

    STORE_BARRIER();

    server_running = true;

    LOG(INFO)<<"The lcore id of the server thread is "<<rte_lcore_id();

    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    void* tag;  // uniquely identifies a request.
    bool ok;
    void* dequeue_output[1];
    int flag;
    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      if(FLAGS_test_rpc){
        GPR_ASSERT(cq_->Next(&tag, &ok));
        GPR_ASSERT(ok);
        static_cast<CallData*>(tag)->Proceed();
      }
      else{
        flag = llring_sc_dequeue(ring_, dequeue_output);
        if(flag != 0){
          continue;
        }
        else{
          LOG(INFO)<<"Dequeu an object from llring";
          LOG(INFO)<<*(static_cast<string*>(dequeue_output[0]));
        }
      }
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  Greeter::AsyncService service_;
  std::unique_ptr<Server> server_;
  struct llring* ring_;
};

void* run_server(void* arg){
  static_cast<ServerImpl*>(arg)->Run();
  return 0;
}

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);

  cout<<FLAGS_boolean_flag<<endl;
  cout<<FLAGS_string_flag<<endl;

  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  nfa_init_dpdk(argv[0]);

  LOG(INFO)<<"Finish dpdk initialization.";

  const int llring_size = 1024;
  int bytes_per_llring = llring_bytes_with_slots(llring_size);
  struct llring* ring = static_cast<struct llring*>(rte_zmalloc(nullptr, bytes_per_llring, 0));
  llring_init(ring, llring_size, 0, 0);
  llring_set_water_mark(ring, ((llring_size >> 3) * 7));

  ServerImpl server(ring);

  std::thread server_thread(run_server, &server);
  server_thread.detach();

  INST_BARRIER();

  while(server_running == false){
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  LOG(INFO)<<"server thread runs";
  LOG(INFO)<<"The lcore id of the master thread is "<<rte_lcore_id();

  string wtf("wtf??");
  llring_sp_enqueue(ring, static_cast<void*>(&wtf));

  while(true){
    std::this_thread::sleep_for(std::chrono::seconds(50));
  }

  return 0;
}
