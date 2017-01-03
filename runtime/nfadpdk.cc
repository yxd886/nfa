#include <cstdio>
#include <string>
#include <cstring>
#include <syslog.h>

#include <rte_config.h>
#include <rte_cycles.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#include <glog/logging.h>

#include "./bessport/packet.h"

#include "nfadpdk.h"

using std::string;

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

struct rte_mempool *mempool;

static void load_mempool(void)
{
  /* Try pframe pool on node 0 first */
  const int BEGIN = 16384;
  const int END = 524288;
  char name[256];
  for (int i = BEGIN; i <= END; i *= 2) {
    sprintf(name, "pframe0_%dk", (i + 1) / 1024);
    mempool = rte_mempool_lookup(name);
    if (mempool)
      return;
  }

  for (int i = BEGIN; i <= END; i *= 2) {
    sprintf(name, "pframe1_%dk", (i + 1) / 1024);
    mempool = rte_mempool_lookup(name);
    if (mempool)
      return;
  }

  assert(0);
}

static void nfa_init_eal(const char* argv0){
  int ret;
  FILE *org_stdout;

  int rte_argc = 0;
  const char *rte_argv[16];

  char opt_master_lcore[1024];
  char opt_lcore_bitmap[1024];

  sprintf(opt_master_lcore, "%d", RTE_MAX_LCORE - 1);
  sprintf(opt_lcore_bitmap, "%d@%d", RTE_MAX_LCORE - 1, 0);

  rte_argv[rte_argc++] = argv0;
  rte_argv[rte_argc++] = "--master-lcore";
  rte_argv[rte_argc++] = opt_master_lcore;
  rte_argv[rte_argc++] = "--lcore";
  rte_argv[rte_argc++] = opt_lcore_bitmap;
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

  load_mempool();
  LOG(INFO)<<"Load Memory Pool in init function";
  struct snbuf *pkt = (struct snbuf *)rte_pktmbuf_alloc(mempool);
  LOG(INFO)<<"Allocation good";
}

void nfa_init_dpdk(const char* argv0){
  nfa_init_eal(argv0);
  int num_of_loaded_mempool = bess::nfa_load_mempool();
  // bess::nfa_try_allocate();
  if(num_of_loaded_mempool == 0){
    LOG(ERROR)<<"Fail to load memory pool";
    exit(EXIT_FAILURE);
  }
}

void init_bess(uint32_t lcore, char *name)
{
  int rte_argc;
  char *rte_argv[16];

  char opt_core_bitmap[256];

  int ret;

  uint64_t cpumask = (1ull << lcore);

  sprintf(opt_core_bitmap, "0x%lx", cpumask);

  rte_argc = 7;
  rte_argv[0] = "";
  rte_argv[1] = "-c";
  rte_argv[2] = opt_core_bitmap;
  rte_argv[3] = "-n";
  rte_argv[4] = "4";  /* number of mem channels (Sandy/Ivy Bridge) */
  rte_argv[5] = "--proc-type";
  rte_argv[6] = "secondary";
  rte_argv[7] = NULL;

  /* reset getopt() */
  optind = 0;

  ret = rte_eal_init(rte_argc, rte_argv);
  assert(ret >= 0);

  load_mempool();
  struct rte_mbuf* mbuf = rte_pktmbuf_alloc(mempool);
  LOG(INFO)<<"Allocation good";

}
