//


#include "actor.h"
#include "../module/message_passing.h"


using namespace std;

void actor::remote_send(int runtime_id, int actor_id, char* msg, int size){

/*
	reliable_p2p *p2p=p2p_find(runtime_id);

	if(p2p==nullptr){
		p2p=p2p_create(runtime_id);
	}

	p2p->interface_to_actor(actor_id,msg,size);
*/

}
