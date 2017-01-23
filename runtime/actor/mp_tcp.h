#ifndef MPTCP_H
#define MPTCP_H

#include "../bessport/packet.h"
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <glog/logging.h>
#include <stdio.h>


struct tcp_header {
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
};


struct mptcp_option {
	uint8_t tcp_kind;
	uint8_t length;
	uint8_t subtype;
	char payload[0];
};

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


bool is_mptcp_flow(bess::Packet* pkt,int32_t runtime_id,uint32_t target_no, int32_t& migration_target_id){



	struct rte_mbuf* rte_pkt=reinterpret_cast<struct rte_mbuf *>(pkt);
	unsigned char *t =rte_pktmbuf_mtod(rte_pkt, unsigned char*);
	char* packet = (char*)t;
	// ethernet header
	ether_hdr *eth_header = (ether_hdr *) packet;
		// ip header
	iphdr *ip_header = (struct iphdr*)(packet + sizeof(struct ether_hdr));//(bytes + sizeof(struct ether_header));

	if(eth_header->ether_type != 0x0800) {  //ip

		LOG(INFO)<<"NOT IP PACKET:";
		return false;
	}

	if(ip_header->version!= 4) {

		LOG(INFO)<<"NOT IPV4 PACKET";
		return false;
	}

	if(ip_header->protocol!= IPPROTO_TCP) {

		LOG(INFO)<<"NOT TCP PACKET";
		return false;
	}

	// TCP packet information
	tcp_header *tcp_header = (struct tcp_header*)(packet + sizeof(struct ether_hdr)+(ip_header->ihl)*4);
	char *option = tcp_header->options;
	char *payload = (char*)((uint32_t*)tcp_header+(tcp_header->data_offset >> 4));

	uint16_t dport =ntohs(tcp_header->dport);

	while(option < payload) {
		uint16_t option_kind = *option;
		//LOG(INFO)<<"option_kind:"<<option_kind;
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
				migration_target_id=(dport%target_no)+3;
				//LOG(INFO)<<"migration_target_id:"<<migration_target_id;
				//LOG(INFO)<<"runtime_id:"<<runtime_id;
				if(migration_target_id!=runtime_id){

					return true;
				}else{

					return false;

				}

			}

			default:
				//fprintf(stderr, "Fatal error: unknown TCP option kind: %u\n", option_kind); exit(-1);
				break;
		}

	}
	return false;


}



#endif
