#include "../../rpc/livesness_check_client.h"
#include <unistd.h>

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


  LOG(INFO)<<"print any key to migrate all flows in r11 to r21";
	getchar();

	LOG(INFO)<<checker_r2_10241.SetMigrationTarget("202.45.128.156",10241,20000);

	LOG(INFO)<<checker_r1_10241.DeleteOutputMac("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10242.DeleteOutputMac("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10243.DeleteOutputMac("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10244.DeleteOutputMac("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10245.DeleteOutputMac("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10246.DeleteOutputMac("202.45.128.155",10241);

	// LOG(INFO)<<checker_r2_10241.DeleteInputMac("202.45.128.154",10241);

	LOG(INFO)<<checker_r2_10241.MigrateTo("202.45.128.156",10241,300000);

	sleep(2);

	LOG(INFO)<<checker_r1_10241.DeleteOutputRt("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10242.DeleteOutputRt("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10243.DeleteOutputRt("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10244.DeleteOutputRt("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10245.DeleteOutputRt("202.45.128.155",10241);
	LOG(INFO)<<checker_r1_10246.DeleteOutputRt("202.45.128.155",10241);

	// LOG(INFO)<<checker_r2_10241.DeleteInputRt("202.45.128.154",10241);

  LOG(INFO)<<"please shutdown the r2 rt1 , then press any key to continue";
	getchar();
  LOG(INFO)<<"please restart r2 rt1, then press any key to continue";
	getchar();

	// LOG(INFO)<<checker_r1_10241.SingleAddOutputRt("202.45.128.155",10241);
	// LOG(INFO)<<checker_r1_10241.AddOutputMac("202.45.128.155",10241);
	// LOG(INFO)<<checker_r2_10241.AddInputMac("202.45.128.154",10241);





  return 0;
}
