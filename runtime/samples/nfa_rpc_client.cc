
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

 private:
  std::unique_ptr<Runtime_RPC::Stub> stub_;
};

int main(int argc, char** argv) {
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  LivenessCheckClient checker(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  std::string reply = checker.Check();
  LOG(INFO)<<reply;

  return 0;
}
