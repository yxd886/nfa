#ifndef MPTCP_H
#define MPTCP_H

#include "../../bessport/packet.h"
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <netinet/if_ether.h>
#include "../mptcp/mp_tcp_fs.h"

typedef struct tcp_header {
	uint16_t sport;
	uint16_t dport;
	uint32_t seq_number;
	uint32_t ack_number;
	uint8_t data_offset;
	uint8_t flags;
	uint16_t window_size;
	uint16_t checksum;
	uint16_t urgent_pointer;
	char options[];
} tcp_header_t;


typedef struct mptcp_option {
	uint8_t tcp_kind;
	uint8_t length;
	uint8_t subtype;
	char payload[0];
} mptcp_option_t;

enum tcp_option_kind {
	TCP_OPTION_EOL        = 0,
	TCP_OPTION_NOP        = 1,
	TCP_OPTION_MSS        = 2,
	TCP_OPTION_WIN_SCALE  = 3,
	TCP_OPTION_SACK_ALLOW = 4,
	TCP_OPTION_SACK       = 5,
	TCP_OPTION_TIMESTAMPS = 8,
	TCP_OPTION_MULTIPATH  = 30,
};
class mp_tcp{
public:
	mp_tcp():target_no(0){

		runtime_id=coordinator::get_runtime_id();
	}



  inline void nf_logic_impl(bess::Packet* pkt, mp_tcp_fs* fs){



		struct rte_mbuf* rte_pkt=reinterpret_cast<struct rte_mbuf *>(pkt);
		unsigned char *t =rte_pktmbuf_mtod(rte_pkt, unsigned char*);
		char* packet = (char*)t;
  	// ethernet header
  	ether_header *eth_header = (ether_header *) packet;
  		// ip header
  		iphdr *ip_header = (struct iphdr*)(packet + sizeof(struct ether_hdr));//(bytes + sizeof(struct ether_header));

  		/*
  		// XXX DEBUG: print packet information
  		static int i=0;
  		printf("\n\n[%d] time=%ld.%06ld, length=%d\n", i, pkt_header->ts.tv_sec, pkt_header->ts.tv_usec, pkt_header->len);
  		i++;
  		// */

  		// discard non-IP packets
  		if(ntohs(eth_header->ether_type) != ETHERTYPE_IP) {
  			//fprintf(stderr, "Ignoring non-IP packets\n");
  			return;
  		}

  		// Discard non IPv4 packets.
  		// TODO: add IPv6 support
  		if(ip_header->version!= 4) {
  		//	fprintf(stderr, "Ignoring non-IPv4 packets.\n");
  			return;
  		}
  		// discard non-TCP packets
  		if(ip_header->protocol!= IPPROTO_TCP) {
  			//fprintf(stderr, "Ignoring non-TCP packets\n");
  			return;
  		}

  		// TCP packet information
  		tcp_header *tcp_header = (struct tcp_header*)(packet + sizeof(struct ether_hdr)+(ip_header->ihl)*4);
  		char *option = tcp_header->options;
  		char *payload = (char*)((uint32_t*)tcp_header+(tcp_header->data_offset >> 4));

  		uint16_t dport =ntohs(tcp_header->dport);
  		uint32_t migration_target=0;





  		while(option < payload) {
  			uint16_t option_kind = *option;
  			uint8_t mptcp_subtype;
  			switch(option_kind) {
  				case TCP_OPTION_EOL:
  				case TCP_OPTION_NOP:
  					option+=1;
  					break;
  				case TCP_OPTION_SACK_ALLOW:
  					option+=2;
  					break;
  				case TCP_OPTION_WIN_SCALE:
  					option+=3;
  					break;
  				case TCP_OPTION_MSS:
  					option+=4;
  					break;
  				case TCP_OPTION_TIMESTAMPS:
  					option+=10;
  					break;
  				case TCP_OPTION_SACK:
  					option+=*(option+1);
  					break;
  				case TCP_OPTION_MULTIPATH:{
  					migration_target=(dport%target_no)+1;
  					if(migration_target!=runtime_id){
  						fs->need_to_migrate=true;
  						fs->migration_target_id=migration_target;
  					}else{

  						fs->need_to_migrate=false;
  					}


  					return;

  				}

  				default:
  					//fprintf(stderr, "Fatal error: unknown TCP option kind: %u\n", option_kind); exit(-1);
  					break;
  			}

  		}


  }

  static void update_target_no(uint32_t target_no){
  	mp_tcp::target_no=target_no;
  }
  static void set_runtime_id(int32_t runtime_id){
  	mp_tcp::runtime_id=runtime_id;
  }

private:


 static int32_t runtime_id;
 static uint32_t target_no;
};

#endif
