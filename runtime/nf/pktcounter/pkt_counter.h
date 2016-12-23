#ifndef PKT_COUNTER_H
#define PKT_COUNTER_H

#include "../../bessport/packet.h"
#include "pkt_counter_fs.h"

class pkt_counter{
public:
  inline void nf_logic_impl(bess::Packet* pkt, pkt_counter_fs* fs){
    fs->counter += 1;
  }
};

#endif
