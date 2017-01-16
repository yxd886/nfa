#include "../rpc/livesness_check_client.h"

int main(int argc, char** argv) {
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  LivenessCheckClient checker_10240(grpc::CreateChannel(
      "localhost:10240", grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_10241(grpc::CreateChannel(
        "localhost:10241", grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_10242(grpc::CreateChannel(
        "localhost:10242", grpc::InsecureChannelCredentials()));

  // LOG(INFO)<<checker_10240.AddOutputRt();
  // LOG(INFO)<<checker_10240.AddOutputMac(10241);
  // LOG(INFO)<<checker_10240.AddOutputRt();
  // LOG(INFO)<<checker_10240.AddOutputMac(10241);
  LOG(INFO)<<checker_10241.AddReplicas(10242);

  return 0;
}
