#ifndef MODIFIED_FIREWALL_HPP
#define MODIFIED_FIREWALL_HPP
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
//#include <rte_ethdev.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <sys/time.h>
#include <deque>
#include <set>
#include <map>
#include <list>
#include <asm-generic/int-ll64.h>
#include <rte_config.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#include "fw_headinfo.h"
#include "../../bessport/packet.h"
#include "firewall_fs.h"
#include <cassert>




class firewall{
public:
  firewall(){
    //FILE*fp=fopen("/home/net/nfa-gpu/runtime/nf/firewall/rule.txt","r");
    char saddr[200];
    memset(saddr,0,sizeof(saddr));
    char daddr[200];
    memset(daddr,0,sizeof(daddr));
  //  if(fp==NULL){
   //  std::cout<<"open file error!"<<std::endl;
 //   }
    struct rule r[60];
   struct rule* rp;
  //  std::cout<<"begin to read rules"<<std::endl;
    for(int i=0;i<20;i++){
  	  rp=&r[i];
        *(unsigned char *)&rp->saddr.addr=i%254;
        *(((unsigned char *)&rp->saddr.addr)+1)=(i+100)%254;
        *(((unsigned char *)&rp->saddr.addr)+2)=0;
        *(((unsigned char *)&rp->saddr.addr)+3)=(i+30)%254;
        rp->saddr.mask=32;
        rp->sport=65535;
        *((unsigned char *)&rp->daddr.addr)=i%254;
        *((unsigned char *)&rp->daddr.addr+1)=75;
        *((unsigned char *)&rp->daddr.addr+2)=0;
        *((unsigned char *)&rp->daddr.addr+3)=109;
        rp->daddr.mask=32;
        rp->dport=i%65535;
        rp->protocol=6;
        rp->action=1;
       rules.push_back(r[i]);

    }

    for(int i=0;i<20;i++){
  	  rp=&r[i+20];
        *(unsigned char *)&rp->saddr.addr=(i+59)%254;
        *(((unsigned char *)&rp->saddr.addr)+1)=(i+44)%254;
        *(((unsigned char *)&rp->saddr.addr)+2)=0;
        *(((unsigned char *)&rp->saddr.addr)+3)=(i+90)%254;
        rp->saddr.mask=32;
        rp->sport=65535;
        *((unsigned char *)&rp->daddr.addr)=(i+54)%254;
        *((unsigned char *)&rp->daddr.addr+1)=75;
        *((unsigned char *)&rp->daddr.addr+2)=0;
        *((unsigned char *)&rp->daddr.addr+3)=109;
        rp->daddr.mask=32;
        rp->dport=i%65535;
        rp->protocol=6;
        rp->action=1;
       rules.push_back(r[i]);

    }
    for(int i=0;i<20;i++){

  	  rp=&r[i+40];
  	  *(unsigned char *)&rp->saddr.addr=(i+52)%254;
        *(((unsigned char *)&rp->saddr.addr)+1)=(i+74)%254;
        *(((unsigned char *)&rp->saddr.addr)+2)=0;
        *(((unsigned char *)&rp->saddr.addr)+3)=(i+40)%254;
        rp->saddr.mask=32;
        rp->sport=65535;
        *((unsigned char *)&rp->daddr.addr)=(i+34)%254;
        *((unsigned char *)&rp->daddr.addr+1)=75;
        *((unsigned char *)&rp->daddr.addr+2)=0;
        *((unsigned char *)&rp->daddr.addr+3)=109;
        rp->daddr.mask=32;
        rp->dport=i%65535;
        rp->protocol=6;
        rp->action=1;
       rules.push_back(r[i]);

    }
 //  std::cout<<"begin to close the rule file !"<<std::endl;
  // fclose(fp);
 //  std::cout<<"close the rule file successfully !"<<std::endl;
  }

  void nf_logic_impl(bess::Packet* pkt, firewall_fs* fs);

private:

  void process(char* packet,firewall_fs* fs);

  void Format(char* packet,struct headinfo* hd);

  Bool CompareID_with_mask(uint32_t addr1, uint32_t addr2, uint8_t mask);

  void filter_local_out(struct headinfo *hd,firewall_fs* sesptr);

  uint16_t GetPort(struct headinfo *hd, int flag);



std::vector <struct rule> rules;

};

#endif
