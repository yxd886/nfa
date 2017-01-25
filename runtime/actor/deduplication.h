#ifndef DEDUPLICATION_H
#define DEDUPLICATION_H

#include "../bessport/packet.h"
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <glog/logging.h>
#include <stdio.h>
#include <string.h>
#include "../nf/httpparser/FormatPacket.h"





bool is_duplicate_packet(bess::Packet* pkt){


	struct rte_mbuf* rte_pkt=reinterpret_cast<struct rte_mbuf *>(pkt);
	unsigned char *t =rte_pktmbuf_mtod(rte_pkt, unsigned char*);
	char* packet = (char*)t;
	CFormatPacket format_packet;
	format_packet.Format(packet);
	char tmp[20];
	char cmp[]="duplicate";
	memset(tmp,0,sizeof(tmp));
	rte_memcpy(tmp,format_packet.GetData(),sizeof(cmp));
	if(strcmp(tmp,cmp)==0){
    LOG(INFO)<<"DUPLICATE PACKET";
		return true;
	}else{
		LOG(INFO)<<"NO DUPLICATE PACKET";
		return false;
	}


}

#endif
