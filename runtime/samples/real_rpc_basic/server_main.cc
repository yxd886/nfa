#include <glog/logging.h>
#include "../../nfadpdk.h"
/*#include <iostream>
#include <cassert>
#include <string>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <rte_config.h>
#include <rte_cycles.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_byteorder.h>
#include <rte_mempool.h>
#include <rte_eal.h>
#include <rte_common.h>

#include <time.h>

using namespace std;

struct rte_mempool *mempool;

static void load_mempool(void)
{

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

struct rte_mbuf rte_mbuf_template;

static void init_template(void)
{
  struct rte_mbuf *mbuf;

  std::cout<<"before allocation"<<std::endl;
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);
  mbuf = rte_pktmbuf_alloc(mempool);
  assert(mbuf!=0);

  assert(mbuf!=0);
  std::cout<<"after allocation"<<std::endl;
  rte_mbuf_template = *mbuf;
  rte_pktmbuf_free(mbuf);
}

void init_bess(uint32_t lcore)
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
  rte_argv[4] = "4";
  rte_argv[5] = "--proc-type";
  rte_argv[6] = "secondary";
  rte_argv[7] = NULL;


  optind = 0;

  ret = rte_eal_init(rte_argc, rte_argv);
  assert(ret >= 0);

  load_mempool();

  init_template();
}*/

int main(int argc, char* argv[]){

  google::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  nfa_init_dpdk(argv[0]);
  LOG(INFO)<<"OK";
}


