#ifndef MAC_LIST_ITEM
#define MAC_LIST_ITEM

#include "cdlist.h"

struct generic_list_item{
  struct cdlist_item list_item;
  uint64_t dst_mac_addr;
  int32_t replica_rtid_;
};

#endif
