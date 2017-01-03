#ifndef MAC_LIST_ITEM
#define MAC_LIST_ITEM

#include "cdlist.h"

struct generic_list_item{
  struct cdlist_item list_item;
  uint64_t dst_mac_addr;
  int32_t replica_rtid_;

  int32_t reliable_rtid;
  int pkt_num;
  uint16_t output_gate;
};

#endif
