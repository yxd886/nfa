#ifndef FLOW_ETHER_HEADER
#define FLOW_ETHER_HEADER

#include <rte_config.h>
#include <rte_ether.h>

struct flow_ether_header{
  int32_t dest_rtid;
  struct ether_hdr ethh;

  inline void init(int32_t rtid, uint64_t dest_mac, uint64_t src_mac){
    dest_rtid = rtid;

    ethh.d_addr = *(reinterpret_cast<struct ether_addr*>(&dest_mac));
    ethh.s_addr = *(reinterpret_cast<struct ether_addr*>(&src_mac));
    ethh.ether_type = 0x0800;
  }

  inline void init(int32_t rtid, struct ether_addr* dest_mac, uint64_t src_mac){
    dest_rtid = rtid;

    ethh.d_addr = *dest_mac;
    ethh.s_addr = *(reinterpret_cast<struct ether_addr*>(&src_mac));
    ethh.ether_type = 0x0800;
  }
};

#endif
