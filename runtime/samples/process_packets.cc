#include <iostream>

#include <rte_config.h>

#include <glog/logging.h>

#include "../nfaflags.h"

using namespace std;

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);

  cout<<FLAGS_boolean_flag<<endl;
  cout<<FLAGS_string_flag<<endl;

  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  DLOG(INFO) << RTE_MAX_LCORE;
}
