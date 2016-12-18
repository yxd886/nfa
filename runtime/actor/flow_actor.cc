#include <glog/logging.h>

#include "flow_actor.h"

void flow_actor::handle_message(pkt_msg_t, bess::Packet* pkt){
  // do nothing
  LOG(INFO)<<"Flow actor "<<this->get_id()<<" is processing packet";
}
