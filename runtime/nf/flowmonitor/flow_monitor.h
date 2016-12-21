#ifndef CAF_NF_FLOW_MONITOR_HPP
#define CAF_NF_FLOW_MONITOR_HPP
#include <rte_config.h>
#include <rte_mbuf.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
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
#include "fm_SessionHash.h"
#include "fm_headinfo.h"
#include "../../bessport/packet.h"


class flow_monitor{
public:
  flow_monitor(){
	  _sesHash.Init();
  }

  flow_monitor_fsHashTable  _sesHash;



	void nf_logic_impl(bess::Packet* pkt, flow_monitor_fs* fs){
		struct rte_mbuf* rte_pkt=reinterpret_cast<struct rte_mbuf *>(pkt);
		unsigned char *t =rte_pktmbuf_mtod(rte_pkt, unsigned char*);
		char* raw_packet = (char*)t;
		process(raw_packet, fs);
		//    printf("total number: %d\nudp number: %d\ntcp number: %d\nicmp number: %d\n",ptr->no_total,ptr->no_tcp,ptr->no_udp,ptr->no_icmp);

	}


  void process(char* raw_packet,flow_monitor_fs* fs){
    struct head_info t;
    struct head_info* hd=&t;
    Format(raw_packet,hd);
    uint32_t srcIp = ntohl(hd->m_pIphdr->saddr);
    uint32_t dstIp = ntohl(hd->m_pIphdr->daddr);
    uint16_t srcPort;
    uint16_t dstPort;
    if(hd->m_pTcphdr==NULL){
       srcPort=0;
       dstPort=0;
    }else{
       srcPort = ntohs(hd->m_pTcphdr->source);
       dstPort = ntohs(hd->m_pTcphdr->dest);
     }

    flow_monitor_fsPtr sesptr = _sesHash.Find(srcIp,dstIp,srcPort,dstPort);

    if(sesptr.get() == NULL)
    {
        //如果不存在，则创建新会话
    	printf("new session created!\n");
    	//getchar();
        sesptr = _sesHash.Create(hd);
        if(sesptr.get() == NULL)
        {
            //log  create session error
            return;
        }
        fs->CreatedTime=sesptr->CreatedTime;
        fs->DstIp=sesptr->DstIp;
        fs->DstPort=sesptr->DstPort;
        fs->RefreshTime=sesptr->RefreshTime;
        fs->SrcIp=sesptr->SrcIp;
        fs->SrcPort=sesptr->SrcPort;
        fs->protocol=sesptr->protocol;
        fs->counter=sesptr->counter;
    }
    if(sesptr->protocol==IPPROTO_TCP){
    	fs->no_tcp++;
        sesptr->no_tcp=fs->no_tcp;

    }else if(sesptr->protocol==IPPROTO_UDP){
    	fs->no_udp++;
    	sesptr->no_udp=fs->no_udp;
     }else if(sesptr->protocol==IPPROTO_ICMP){
    	 fs->no_icmp++;
    	 sesptr->no_icmp=fs->no_icmp;
      }
    fs->no_total++;
    fs->counter++;
    sesptr->no_total=fs->no_total;
    sesptr->counter=fs->counter;

    printf("total number: %d\nudp number: %d\ntcp number: %d\nicmp number: %d\n",fs->no_total,fs->no_tcp,fs->no_udp,fs->no_icmp);

  }

    void Format(char* packet,struct head_info* hd){
    hd->m_pEthhdr = (struct ether_hdr*)packet;
    hd->m_pIphdr = (struct iphdr*)(packet + sizeof(struct ether_hdr));
    hd->m_pTcphdr = (struct tcphdr*)(packet + sizeof(struct ether_hdr)+(hd->m_pIphdr->ihl)*4);
    hd->protocol =  hd->m_pIphdr->protocol;
    return;
  }


};

#endif
