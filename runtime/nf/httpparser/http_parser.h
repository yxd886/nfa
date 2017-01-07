
#ifndef CAF_NF_HTTP_PARSER_HPP
#define CAF_NF_HTTP_PARSER_HPP

#include "Public.h"
#include "Receiver.h"
#include "SessionHash.h"
#include <netinet/ip6.h>
#include "SessionHash.h"
#include "../../bessport/packet.h"

#include <glog/logging.h>

class http_parser{
public:
  http_parser(){
    rcv = Receiver();
  }

  void nf_logic_impl(bess::Packet* pkt,http_parser_fs* fs){
    // LOG(INFO)<<"http_parser processing logic is called";
		struct rte_mbuf* rte_pkt=reinterpret_cast<struct rte_mbuf *>(pkt);
		unsigned char *t =rte_pktmbuf_mtod(rte_pkt, unsigned char*);
		char* raw_packet = (char*)t;
		process(raw_packet,fs);

  }


private:

  void process(char* raw_packet,http_parser_fs* fs){


	  rcv.Work(raw_packet,fs);

  }


  Receiver  rcv;
};


#endif
