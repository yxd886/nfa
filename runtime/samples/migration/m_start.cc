
#include "../../rpc/livesness_check_client.h"


int main(int argc, char** argv) {
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  LivenessCheckClient checker_r1_10241(grpc::CreateChannel(
      "202.45.128.154:10241", grpc::InsecureChannelCredentials()));

  LivenessCheckClient checker_r1_10242(grpc::CreateChannel(
      "202.45.128.154:10242", grpc::InsecureChannelCredentials()));

  LivenessCheckClient checker_r1_10243(grpc::CreateChannel(
      "202.45.128.154:10243", grpc::InsecureChannelCredentials()));

  LivenessCheckClient checker_r1_10244(grpc::CreateChannel(
      "202.45.128.154:10244", grpc::InsecureChannelCredentials()));

  LivenessCheckClient checker_r1_10245(grpc::CreateChannel(
      "202.45.128.154:10245", grpc::InsecureChannelCredentials()));

  LivenessCheckClient checker_r1_10246(grpc::CreateChannel(
      "202.45.128.154:10246", grpc::InsecureChannelCredentials()));


 LivenessCheckClient checker_r2_10241(grpc::CreateChannel(
      "202.45.128.155:10241", grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r2_10242(grpc::CreateChannel(
      "202.45.128.155:10242", grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r2_10243(grpc::CreateChannel(
      "202.45.128.155:10243", grpc::InsecureChannelCredentials()));

  LivenessCheckClient checker_r3_10241(grpc::CreateChannel(
      "202.45.128.156:10241", grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r3_10242(grpc::CreateChannel(
      "202.45.128.156:10242", grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r3_10243(grpc::CreateChannel(
      "202.45.128.156:10243", grpc::InsecureChannelCredentials()));
  // Test set replication
  LOG(INFO)<<"r2 rt1"<<checker_r2_10241.Check();
  LOG(INFO)<<"r2 rt2"<<checker_r2_10242.Check();
  LOG(INFO)<<"r2 rt3"<<checker_r2_10243.Check();

  LOG(INFO)<<"r3 rt1"<<checker_r3_10241.Check();
  LOG(INFO)<<"r3 rt2"<<checker_r3_10242.Check();
  LOG(INFO)<<"r3 rt3"<<checker_r3_10243.Check();

  LOG(INFO)<<checker_r2_10241.SetMigrationTarget("202.45.128.156",10241,1000);
  LOG(INFO)<<checker_r2_10241.MigrateTo("202.45.128.156",10241,25000);

  LOG(INFO)<<checker_r2_10242.SetMigrationTarget("202.45.128.156",10242,1000);
  LOG(INFO)<<checker_r2_10242.MigrateTo("202.45.128.156",10242,25000);

  // LOG(INFO)<<checker_r2_10243.SetMigrationTarget("202.45.128.156",10243,1000);
  // LOG(INFO)<<checker_r2_10243.MigrateTo("202.45.128.156",10243,50000);

 /* LOG(INFO)<<checker_r3_10241.SetMigrationTarget("202.45.128.155",10241,1000);
  LOG(INFO)<<checker_r3_10241.MigrateTo("202.45.128.155",10241,50000);

  LOG(INFO)<<checker_r3_10242.SetMigrationTarget("202.45.128.155",10242,1000);
  LOG(INFO)<<checker_r3_10242.MigrateTo("202.45.128.155",10242,50000);

  LOG(INFO)<<checker_r3_10243.SetMigrationTarget("202.45.128.155",10243,1000);
  LOG(INFO)<<checker_r3_10243.MigrateTo("202.45.128.155",10243,50000);
*/

  /*checker_r1_10241.SingleAddOutputRt("202.45.128.155",10241);
  checker_r1_10241.AddOutputMac("202.45.128.155",10241);

  checker_r1_10241.SingleAddOutputRt("202.45.128.155",10242);
  checker_r1_10241.AddOutputMac("202.45.128.155",10242);

  checker_r1_10241.SingleAddOutputRt("202.45.128.155",10243);
  checker_r1_10241.AddOutputMac("202.45.128.155",10243);

  checker_r1_10241.SingleAddOutputRt("202.45.128.156",10241);
  checker_r1_10241.AddOutputMac("202.45.128.156",10241);

  checker_r1_10241.SingleAddOutputRt("202.45.128.156",10242);
  checker_r1_10241.AddOutputMac("202.45.128.156",10242);

  checker_r1_10241.SingleAddOutputRt("202.45.128.156",10243);
  checker_r1_10241.AddOutputMac("202.45.128.156",10243);

  checker_r1_10242.SingleAddOutputRt("202.45.128.155",10241);
  checker_r1_10242.AddOutputMac("202.45.128.155",10241);

  checker_r1_10242.SingleAddOutputRt("202.45.128.155",10242);
  checker_r1_10242.AddOutputMac("202.45.128.155",10242);

  checker_r1_10242.SingleAddOutputRt("202.45.128.155",10243);
  checker_r1_10242.AddOutputMac("202.45.128.155",10243);

  checker_r1_10242.SingleAddOutputRt("202.45.128.156",10241);
  checker_r1_10242.AddOutputMac("202.45.128.156",10241);

  checker_r1_10242.SingleAddOutputRt("202.45.128.156",10242);
  checker_r1_10242.AddOutputMac("202.45.128.156",10242);

  checker_r1_10242.SingleAddOutputRt("202.45.128.156",10243);
  checker_r1_10242.AddOutputMac("202.45.128.156",10243);

  checker_r1_10243.SingleAddOutputRt("202.45.128.155",10241);
  checker_r1_10243.AddOutputMac("202.45.128.155",10241);

  checker_r1_10243.SingleAddOutputRt("202.45.128.155",10242);
  checker_r1_10243.AddOutputMac("202.45.128.155",10242);

  checker_r1_10243.SingleAddOutputRt("202.45.128.155",10243);
  checker_r1_10243.AddOutputMac("202.45.128.155",10243);

  checker_r1_10243.SingleAddOutputRt("202.45.128.156",10241);
  checker_r1_10243.AddOutputMac("202.45.128.156",10241);

  checker_r1_10243.SingleAddOutputRt("202.45.128.156",10242);
  checker_r1_10243.AddOutputMac("202.45.128.156",10242);

  checker_r1_10243.SingleAddOutputRt("202.45.128.156",10243);
  checker_r1_10243.AddOutputMac("202.45.128.156",10243);

  checker_r1_10244.SingleAddOutputRt("202.45.128.155",10241);
  checker_r1_10244.AddOutputMac("202.45.128.155",10241);

  checker_r1_10244.SingleAddOutputRt("202.45.128.155",10242);
  checker_r1_10244.AddOutputMac("202.45.128.155",10242);

  checker_r1_10244.SingleAddOutputRt("202.45.128.155",10243);
  checker_r1_10244.AddOutputMac("202.45.128.155",10243);

  checker_r1_10244.SingleAddOutputRt("202.45.128.156",10241);
  checker_r1_10244.AddOutputMac("202.45.128.156",10241);

  checker_r1_10244.SingleAddOutputRt("202.45.128.156",10242);
  checker_r1_10244.AddOutputMac("202.45.128.156",10242);

  checker_r1_10244.SingleAddOutputRt("202.45.128.156",10243);
  checker_r1_10244.AddOutputMac("202.45.128.156",10243);

  checker_r1_10245.SingleAddOutputRt("202.45.128.155",10241);
  checker_r1_10245.AddOutputMac("202.45.128.155",10241);

  checker_r1_10245.SingleAddOutputRt("202.45.128.155",10242);
  checker_r1_10245.AddOutputMac("202.45.128.155",10242);

  checker_r1_10245.SingleAddOutputRt("202.45.128.155",10243);
  checker_r1_10245.AddOutputMac("202.45.128.155",10243);

  checker_r1_10245.SingleAddOutputRt("202.45.128.156",10241);
  checker_r1_10245.AddOutputMac("202.45.128.156",10241);

  checker_r1_10245.SingleAddOutputRt("202.45.128.156",10242);
  checker_r1_10245.AddOutputMac("202.45.128.156",10242);

  checker_r1_10245.SingleAddOutputRt("202.45.128.156",10243);
  checker_r1_10245.AddOutputMac("202.45.128.156",10243);

  checker_r1_10246.SingleAddOutputRt("202.45.128.155",10241);
  checker_r1_10246.AddOutputMac("202.45.128.155",10241);

  checker_r1_10246.SingleAddOutputRt("202.45.128.155",10242);
  checker_r1_10246.AddOutputMac("202.45.128.155",10242);

  checker_r1_10246.SingleAddOutputRt("202.45.128.155",10243);
  checker_r1_10246.AddOutputMac("202.45.128.155",10243);

  checker_r1_10246.SingleAddOutputRt("202.45.128.156",10241);
  checker_r1_10246.AddOutputMac("202.45.128.156",10241);

  checker_r1_10246.SingleAddOutputRt("202.45.128.156",10242);
  checker_r1_10246.AddOutputMac("202.45.128.156",10242);

  checker_r1_10246.SingleAddOutputRt("202.45.128.156",10243);
  checker_r1_10246.AddOutputMac("202.45.128.156",10243);*/

  return 0;
}
