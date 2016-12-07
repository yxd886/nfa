//
#include "actor.h"
#include "message_passing.h"

void fillpacket(PacketBatch* batch,char* msg, int size){

	int i=0;

	while(true){
		Packet *pkt;
	  char *p;
	  if (!(pkt = bess::Packet::Alloc())) {
	    return nullptr;
	  }

	  p = reinterpret_cast<char *>(pkt->buffer()) +
	      static_cast<size_t>(SNBUF_HEADROOM);
	  if (!p) {
	    return nullptr;
	  }


	  if(size<SNBUF_DATA){
		  pkt->set_data_off(SNBUF_HEADROOM);
		  pkt->set_total_len(size);
		  pkt->set_data_len(size);
		  memcpy_sloppy(p, msg, size);
		  batch->add(pkt);
		  break;
	  }else{
		  pkt->set_data_off(SNBUF_HEADROOM);
		  pkt->set_total_len(SNBUF_DATA);
		  pkt->set_data_len(SNBUF_DATA);
		  memcpy_sloppy(p, msg, SNBUF_DATA);
		  msg+=static_cast<size_t>(SNBUF_DATA);
		  batch->add(pkt);
		  size-=SNBUF_DATA;
	  }

	}

}
void actor::remote_send(int runtime_id, int actor_id, char* msg, int size){

	PacketBatch batch;
	fillpacket(&batch,msg,size);
	reliable_p2p *p2p=p2p_find(runtime_id,actor_id);

	if(p2p==nullptr){
		p2p=p2p_create(runtime_id,actor_id);
	}

	p2p->send(&batch);


}
