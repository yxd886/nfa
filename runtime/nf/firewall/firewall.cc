#include "firewall.h"


void firewall::nf_logic_impl(bess::Packet* pkt, firewall_fs* fs){


	struct rte_mbuf* rte_pkt=reinterpret_cast<struct rte_mbuf *>(pkt);
	unsigned char *t =rte_pktmbuf_mtod(rte_pkt, unsigned char*);
	char* raw_packet = (char*)t;
	process(raw_packet,fs);
}



void firewall::process(char* packet,firewall_fs* fs){
firewall_fs* fws=dynamic_cast<firewall_fs*>(fs);
struct headinfo t;
  struct headinfo* hd=&t;
  Format(packet,hd);
  filter_local_out(hd,fws);
}



void firewall::Format(char* packet,struct headinfo* hd){
  hd->m_pEthhdr = (struct ether_hdr*)packet;
  hd->m_pIphdr = (struct iphdr*)(packet + sizeof(struct ether_hdr));
  if(hd->m_pIphdr->protocol==IPPROTO_TCP){
         hd->m_pTcphdr = (struct tcphdr*)(packet + sizeof(struct ether_hdr)+(hd->m_pIphdr->ihl)*4);
         hd->m_pUdphdr=NULL;
  }else if(hd->m_pIphdr->protocol==IPPROTO_UDP){
     hd->m_pTcphdr = NULL;
     hd->m_pUdphdr=(struct udphdr*)(packet + sizeof(struct ether_hdr)+(hd->m_pIphdr->ihl)*4);
   }else{
      hd->m_pTcphdr = NULL;
      hd->m_pUdphdr=NULL;
    }

  hd->protocol =  hd->m_pIphdr->protocol;
  return;
}



Bool firewall::CompareID_with_mask(uint32_t addr1, uint32_t addr2, uint8_t mask){
  uint32_t addr1_temp, addr2_temp;
  Bool flag = false;
  addr1_temp = ntohl(addr1);
  addr2_temp = ntohl(addr2);

  addr1_temp = MASK_IP(addr1_temp, mask);
  addr2_temp = MASK_IP(addr2_temp, mask);

  flag = (addr1_temp == addr2_temp);


  return flag;
}



void firewall::filter_local_out(struct headinfo *hd,firewall_fs* sesptr){
  uint32_t s_addr, d_addr;
  uint8_t protocol;
  uint16_t s_port, d_port;
  char strtime[128]= {0};
  time_t t;
  tm* local;
  Bool match = false;
  Bool flag = false;
  protocol = hd->protocol;
  s_addr = hd->m_pIphdr->saddr;
  d_addr = hd->m_pIphdr->daddr;
   sesptr->counter++;
  s_port = GetPort(hd, SRC);
  d_port = GetPort(hd, DEST);
  std::vector<struct rule>::iterator ptr;
  for(ptr=rules.begin();ptr!=rules.end();ptr++){
    match = false;
    match = (ptr->saddr.addr == ANY_ADDR ? true : CompareID_with_mask(ptr->saddr.addr,s_addr,ptr->saddr.mask));
    if(!match){

      continue;
    }
    match = (ptr->daddr.addr == ANY_ADDR ? true : CompareID_with_mask(ptr->daddr.addr,d_addr,ptr->daddr.mask));
    if(!match){
      continue;
    }
    match = (ptr->protocol == ANY_PROTOCOL) ? true : (ptr->protocol == protocol);
    if(!match){
      continue;
    }
    match = (ptr->sport == ANY_PORT) ? true : (ptr->sport == s_port);
    if(!match){
      continue;
    }
    match = (ptr->dport == ANY_PORT) ? true : (ptr->dport == d_port);
    if(!match){
      continue;
    }
  //  match = ptr->action ? 0 : 1;

    if(match){
      flag = ptr->action?false:true;
      ++sesptr->match_no;
      break;
    }
    else{
      flag = false;
      break;
    }
  }//loop for match rule
  if(flag){
    sesptr->drop_no++;
    sesptr->current_pass=false;
  }else{
    sesptr->pass_no++;
    sesptr->current_pass=true;
   }

}


 uint16_t firewall::GetPort(struct headinfo *hd, int flag){
	uint16_t port = ANY_PORT;
	switch(hd->m_pIphdr->protocol){
		case IPPROTO_TCP:
			if(flag == SRC)
				port = ntohs(hd->m_pTcphdr->source);
			else if(flag == DEST)
				port = ntohs(hd->m_pTcphdr->dest);
			break;
		case IPPROTO_UDP:
			if(flag == SRC)
				port = ntohs(hd->m_pUdphdr->source);
			else if(flag == DEST)
				port = ntohs(hd->m_pUdphdr->dest);
			break;
		default:
			port = ANY_PORT;
	}
	return port;
 }
