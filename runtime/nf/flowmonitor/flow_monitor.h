#ifndef CAF_NF_FLOW_MONITOR_HPP
#define CAF_NF_FLOW_MONITOR_HPP
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

	  if(fs==NULL){
      printf("flow monitor dynamic cast fail\n");
	  }else{
		  printf("flow monitor dynamic cast successful\n");
	  }
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

  template <class Inspector>
  static typename Inspector::result_type inspect(Inspector& f,  flow_monitor_fs& x) {
  return f(meta::type_name("pktcs"), x.SrcIp,x.DstIp,x.SrcPort,x.DstPort,x.protocol,x.CreatedTime,
                     x.RefreshTime,x.no_tcp,x.no_udp,x.no_icmp,x.no_total,x,counter, x.nf_type_sig);
}

void serialize(actor_system& sys, vector<char>& buf, flow_state* fs){
  assert(fs->nf_type_sig == nf_type_sig);
  flow_monitor_fs* pc_fs = dynamic_cast<flow_monitor_fs*>(fs);
  binary_serializer bs{sys, buf};
  auto e = bs(*pc_fs);
  if (e) {
    std::cerr << "*** unable to serialize foo2: "
              << sys.render(e) << std::endl;
    return;
  }
}

  unique_ptr<flow_state>deserialize(actor_system& sys, vector<char>& buf){
  binary_deserializer ds{sys, buf};
   flow_monitor_fs* fs_ptr = new flow_monitor_fs(this->nf_type_sig);
  auto e = ds(*fs_ptr);
  return unique_ptr<flow_state>(fs_ptr);
}

};

#endif
#ifndef CAF_NF_FLOW_MONITOR_HPP
#define CAF_NF_FLOW_MONITOR_HPP
#include "actor/network_function.hpp"
#include "actor/flow_state.hpp"
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




class flow_monitor: public network_function{
public:
  flow_monitor(uint8_t nf_type_sig):network_function(nf_type_sig){
	  _sesHash.Init();
  }

  flow_monitor_fsHashTable  _sesHash;

  void init(){
  }


  unique_ptr<flow_state> allocate_flow_state(){
	  unique_ptr<flow_state> f (new flow_monitor_fs(nf_type_sig));
	  return f;
  }


 bool process_pkt(struct rte_mbuf* rte_pkt,flow_state* fs){
      assert(fs->nf_type_sig == nf_type_sig);
      unsigned char *t =rte_pktmbuf_mtod(rte_pkt, unsigned char*);
      char* raw_packet = (char*)t;
      process(raw_packet, fs);
      return true;
  //    printf("total number: %d\nudp number: %d\ntcp number: %d\nicmp number: %d\n",ptr->no_total,ptr->no_tcp,ptr->no_udp,ptr->no_icmp);

  }


  void process(char* raw_packet,flow_state* fs){
    struct head_info t;
    struct head_info* hd=&t;
    Format(raw_packet,hd);

    flow_monitor_fs* fms=dynamic_cast<flow_monitor_fs*>(fs);
	  if(fms==NULL){
      printf("flow monitor dynamic cast fail\n");
	  }else{
		  printf("flow monitor dynamic cast successful\n");
	  }
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
        fms->CreatedTime=sesptr->CreatedTime;
        fms->DstIp=sesptr->DstIp;
        fms->DstPort=sesptr->DstPort;
        fms->RefreshTime=sesptr->RefreshTime;
        fms->SrcIp=sesptr->SrcIp;
        fms->SrcPort=sesptr->SrcPort;
        fms->protocol=sesptr->protocol;
        fms->counter=sesptr->counter;
    }
    if(sesptr->protocol==IPPROTO_TCP){
    	fms->no_tcp++;
        sesptr->no_tcp=fms->no_tcp;

    }else if(sesptr->protocol==IPPROTO_UDP){
    	fms->no_udp++;
    	sesptr->no_udp=fms->no_udp;
     }else if(sesptr->protocol==IPPROTO_ICMP){
    	 fms->no_icmp++;
    	 sesptr->no_icmp=fms->no_icmp;
      }
    fms->no_total++;
    fms->counter++;
    sesptr->no_total=fms->no_total;
    sesptr->counter=fms->counter;

    printf("total number: %d\nudp number: %d\ntcp number: %d\nicmp number: %d\n",fms->no_total,fms->no_tcp,fms->no_udp,fms->no_icmp);

  }

    void Format(char* packet,struct head_info* hd){
    hd->m_pEthhdr = (struct ether_hdr*)packet;
    hd->m_pIphdr = (struct iphdr*)(packet + sizeof(struct ether_hdr));
    hd->m_pTcphdr = (struct tcphdr*)(packet + sizeof(struct ether_hdr)+(hd->m_pIphdr->ihl)*4);
    hd->protocol =  hd->m_pIphdr->protocol;
    return;
  }

  template <class Inspector>
  static typename Inspector::result_type inspect(Inspector& f,  flow_monitor_fs& x) {
  return f(meta::type_name("pktcs"), x.SrcIp,x.DstIp,x.SrcPort,x.DstPort,x.protocol,x.CreatedTime,
                     x.RefreshTime,x.no_tcp,x.no_udp,x.no_icmp,x.no_total,x,counter, x.nf_type_sig);
}

void serialize(actor_system& sys, vector<char>& buf, flow_state* fs){
  assert(fs->nf_type_sig == nf_type_sig);
  flow_monitor_fs* pc_fs = dynamic_cast<flow_monitor_fs*>(fs);
  binary_serializer bs{sys, buf};
  auto e = bs(*pc_fs);
  if (e) {
    std::cerr << "*** unable to serialize foo2: "
              << sys.render(e) << std::endl;
    return;
  }
}

  unique_ptr<flow_state>deserialize(actor_system& sys, vector<char>& buf){
  binary_deserializer ds{sys, buf};
   flow_monitor_fs* fs_ptr = new flow_monitor_fs(this->nf_type_sig);
  auto e = ds(*fs_ptr);
  return unique_ptr<flow_state>(fs_ptr);
}

};

#endif
