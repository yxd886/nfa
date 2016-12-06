#include <iostream>
#include <cstdio>

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

  int rte_argc = 0;
  const char *rte_argv[16];

  char opt_master_lcore[1024];
  char opt_lcore_bitmap[1024];

  sprintf(opt_master_lcore, "%d", RTE_MAX_LCORE - 1);
  sprintf(opt_lcore_bitmap, "%d@%d", RTE_MAX_LCORE - 1, 0);

  rte_argv[rte_argc++] = argv[0];
  rte_argv[rte_argc++] = "--master-lcore";
  rte_argv[rte_argc++] = opt_master_lcore;
  rte_argv[rte_argc++] = "--lcore";
  rte_argv[rte_argc++] = opt_lcore_bitmap;
  rte_argv[rte_argc++] = "-n";
  rte_argv[rte_argc++] = "4";
  rte_argv[rte_argc++] = "--proc-type";
  rte_argv[rte_argc++] = "secondary";
  rte_argv[rte_argc] = nullptr;

  int ret = rte_eal_init(rte_argc, const_cast<char **>(rte_argv));

  if (ret < 0) {
    LOG(ERROR) << "rte_eal_init() failed: ret = " << ret;
    exit(EXIT_FAILURE);
  }

  LOG(INFO) << "DPDK EAL finishes initialization";
}
