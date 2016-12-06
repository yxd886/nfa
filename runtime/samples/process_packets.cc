#include <iostream>

#include <glog/logging.h>

#include "../nfaflags.h"

using namespace std;

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);

  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "Found " << 100 << " cookies";

  cout<<FLAGS_boolean_flag<<endl;
  cout<<FLAGS_string_flag<<endl;
}
