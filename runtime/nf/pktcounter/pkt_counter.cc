#include "pkt_counter.h"

void pkt_counter::nf_logic_impl(bess::Packet* pkt, pkt_counter_fs* fs){
  fs->counter += 1;
}
