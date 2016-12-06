#include <iostream>

#include <glog/logging.h>

#include "../nfaflags.h"

using namespace std;

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);

  cout<<FLAGS_boolean_flag<<endl;
  cout<<FLAGS_string_flag<<endl;

  FLAGS_log_dir = "/some/log/directory";
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "file";

   LOG(INFO) << "stderr";
   LOG(INFO) << "the same file";
}
