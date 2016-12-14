#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <rte_config.h>
#include <rte_lcore.h>
#include <rte_malloc.h>

#include <glog/logging.h>

#include "../nfaflags.h"
#include "../nfadpdk.h"
#include "../bessport/worker.h"
#include "../bessport/tc.h"
#include "../bessport/task.h"
#include "../bessport/module.h"
#include "../bessport/mem_alloc.h"

using namespace std;

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);

  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  nfa_init_dpdk(argv[0]);

  LOG(INFO)<<"Finish dpdk initialization.";
}
