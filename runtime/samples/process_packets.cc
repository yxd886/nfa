#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <syslog.h>
#include <unistd.h>

#include <rte_config.h>
#include <rte_config.h>
#include <rte_cycles.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>

#include <glog/logging.h>

#include "../nfaflags.h"

using namespace std;

static void disable_syslog() {
  setlogmask(0x01);
}

static void enable_syslog() {
  setlogmask(0xff);
}

static ssize_t dpdk_log_init_writer(void *, const char *data, size_t len) {
  enable_syslog();
  LOG(INFO) << std::string(data, len);
  disable_syslog();
  return len;
}

static ssize_t dpdk_log_writer(void *, const char *data, size_t len) {
  LOG(INFO) << std::string(data, len);
  return len;
}

static void nfa_init_eal(char* argv0){
  int ret;
  FILE *org_stdout;

  int rte_argc = 0;
  const char *rte_argv[16];

  char opt_master_lcore[1024];
  char opt_lcore_bitmap[1024];

  sprintf(opt_master_lcore, "%d", RTE_MAX_LCORE - 1);
  sprintf(opt_lcore_bitmap, "%d@%d", RTE_MAX_LCORE - 1, 0);

  rte_argv[rte_argc++] = argv0;
  //rte_argv[rte_argc++] = "--master-lcore";
  //rte_argv[rte_argc++] = opt_master_lcore;
  //rte_argv[rte_argc++] = "--lcore";
  //rte_argv[rte_argc++] = opt_lcore_bitmap;
  sprintf(opt_master_lcore, "%d", (int)FLAGS_temp_core);
  rte_argv[rte_argc++] = "-c";
  rte_argv[rte_argc++] = opt_master_lcore;
  rte_argv[rte_argc++] = "-n";
  rte_argv[rte_argc++] = "4";
  rte_argv[rte_argc++] = "--proc-type";
  rte_argv[rte_argc++] = "secondary";
  rte_argv[rte_argc] = nullptr;

  cookie_io_functions_t dpdk_log_init_funcs;
  cookie_io_functions_t dpdk_log_funcs;

  std::memset(&dpdk_log_init_funcs, 0, sizeof(dpdk_log_init_funcs));
  std::memset(&dpdk_log_funcs, 0, sizeof(dpdk_log_funcs));

  dpdk_log_init_funcs.write = &dpdk_log_init_writer;
  dpdk_log_funcs.write = &dpdk_log_writer;

  org_stdout = stdout;
  stdout = fopencookie(nullptr, "w", dpdk_log_init_funcs);

  disable_syslog();

  ret = rte_eal_init(rte_argc, const_cast<char **>(rte_argv));

  if (ret < 0) {
    LOG(ERROR) << "rte_eal_init() failed: ret = " << ret;
    exit(EXIT_FAILURE);
  }

  enable_syslog();
  fclose(stdout);
  stdout = org_stdout;

  rte_openlog_stream(fopencookie(nullptr, "w", dpdk_log_funcs));

  LOG(INFO) << "DPDK EAL finishes initialization";
}

static struct rte_mempool *nfa_pframe_pool[RTE_MAX_NUMA_NODES];

static void nfa_load_mempool(){
  int i,j;
  int BEGIN = 16384;
  int END = 524288;
  char name[256];

  int initialized[RTE_MAX_NUMA_NODES];
  for (i = 0; i < RTE_MAX_NUMA_NODES; i++) {
    initialized[i] = 0;
  }

  for (i = 0; i < RTE_MAX_LCORE; i++) {
    int sid = rte_lcore_to_socket_id(i);

    if (!initialized[sid]) {

      for (j = BEGIN; j <= END; j *= 2) {
        sprintf(name, "pframe%d_%dk", sid, (j + 1) / 1024);
        nfa_pframe_pool[sid] = rte_mempool_lookup(name);

        if (nfa_pframe_pool[sid]){
          LOG(INFO) << "Find a memory pool on lcore "<<sid<<" with name "<<string(name);
          break;
        }
      }

      if(j>END){
        LOG(ERROR)<<"Fail to find a memory pool on lcore "<<sid;
        exit(EXIT_FAILURE);
      }

      initialized[sid] = 1;
    }
  }

  LOG(INFO) << "Finish loading the memory pool";
}

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);

  cout<<FLAGS_boolean_flag<<endl;
  cout<<FLAGS_string_flag<<endl;

  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  nfa_init_eal(argv[0]);

  nfa_load_mempool();

  if((string(FLAGS_input_port)=="")||(String(FLAGS_output_port)=="")){
    LOG(ERROR)<<"The name of intput/output port must be specified";
    exit(EXIT_FAILURE);
  }


}
