
#ifndef CAF_NF_IDS_HPP
#define CAF_NF_IDS_HPP

#include "../httpparser/Public.h"
#include "Receiver.h"
#include "SessionHash.h"
#include <netinet/ip6.h>
#include "SessionHash.h"
#include "../../bessport/packet.h"

#include <glog/logging.h>

class ids{
public:
  ids(){
    rcv = Ids_Receiver();
  }

  void nf_logic_impl(bess::Packet* pkt,ids_fs* fs){
    // LOG(INFO)<<"ids processing logic is called";
		struct rte_mbuf* rte_pkt=reinterpret_cast<struct rte_mbuf *>(pkt);
		unsigned char *t =rte_pktmbuf_mtod(rte_pkt, unsigned char*);
		char* raw_packet = (char*)t;
		process(raw_packet,fs);

  }


private:

  void process(char* raw_packet,ids_fs* fs){


	  rcv.Work(raw_packet,fs);

  }


  Ids_Receiver  rcv;
};


#endif
