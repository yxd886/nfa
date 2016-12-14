#ifndef NETWORK_FUNCTION_HPP
#define NETWORK_FUNCTION_HPP

#include "flow_state.h"
#include <memory>

#include <rte_config.h>
#include <rte_mbuf.h>

//#include "caf/all.hpp"

//using namespace caf;

using std::unique_ptr;

using network_function_t = uint8_t;

using service_chain_t = uint64_t;

static network_function_t
compute_network_function(service_chain_t s, int pos){
  return static_cast<uint8_t>((s>>(8*pos))&0x00000000000000FF);
}

static int compute_service_chain_length(service_chain_t s){
  int length = 0;
  bool encounter_zero = false;
  for(int i=0; i<8; i++){
    network_function_t nf =
        static_cast<uint8_t>((s>>(8*i))&0x00000000000000FF);
    if(nf>0){
      length+=1;
      if(encounter_zero){
        return -1;
      }
    }
    else{
      encounter_zero = true;
    }
  }
  return length;
}

class network_function{
public:
  explicit network_function(uint8_t nf_type_sig) : nf_type_sig(nf_type_sig){};
  virtual ~network_function() = default;

  virtual void init() = 0;

  virtual unique_ptr<flow_state> allocate_flow_state() = 0;

  virtual bool process_pkt(struct rte_mbuf* pkt, flow_state* fs) = 0;

  virtual void serialize(actor_system& sys, vector<char>& buf, flow_state* fs) = 0;
  virtual unique_ptr<flow_state> deserialize(actor_system& sys, vector<char>& buf) = 0;

  uint8_t nf_type_sig;
};

#endif
