#include <iostream>

#include <glog/logging.h>

#include "../nfaflags.h"

using namespace std;

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);

  cout<<FLAGS_boolean_flag<<endl;
  cout<<FLAGS_string_flag<<endl;

  LOG(INFO) << "file";
  // Most flags work immediately after updating values.
  FLAGS_logtostderr = 1;
  LOG(INFO) << "stderr";
  FLAGS_logtostderr = 0;
  // This won't change the log destination. If you want to set this
  // value, you should do this before google::InitGoogleLogging .
  FLAGS_log_dir = "./";
  LOG(INFO) << "the same file";
}
