#ifndef PKT_COUNTER_H
#define PKT_COUNTER_H

#include "../../bessport/packet.h"
#include "load_balancer_fs.h"

class load_balancer{
public:

	load_balancer():ip(1){}
  inline void nf_logic_impl(bess::Packet* pkt, load_balancer_fs* fs){
    if(fs->is_init==false){
    	fs->is_init=true;
    	fs->ip=ip++;
    }

    uint32_t* ip_dst = pkt->head_data<uint32_t*>(14+16);
    *ip_dst=htonl(fs->ip);
  }


private:
  uint32_t ip;
};

#endif
