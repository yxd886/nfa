#ifndef NF_ITEM_H
#define NF_ITEM_H

#include "network_function_base.h"

struct nf_item{
  network_function_base* nf;
  char* nf_flow_state_ptr;
  size_t nf_flow_state_size;
};

#endif
