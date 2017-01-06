#include "../mptcp/mp_tcp.h"

#include "../base/network_function_register.h"

bool registered_mp_tcp =
    static_nf_register::get_register().register_nf<mp_tcp, mp_tcp_fs>("mp_tcp", 5);



void mp_tcp::Format(char* packet,struct headinfo* hd){
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
