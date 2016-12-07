#ifndef NETWORK_FUNCTION_HUB
#define NETWORK_FUNCTION_HUB

#include "nfs/pkt_counter.hpp"
#include "nfs/modified_firewall.hpp"
#include <map>
#include <memory>
#include <tuple>

using std::map;
using std::make_pair;
using std::unique_ptr;

static constexpr uint8_t pkt_counter_type = 1;
static constexpr uint8_t firewall_type = 2;

class network_function_hub{
public:
  network_function_hub(actor_system& sys):sys(sys){};

  void init(){
    unique_ptr<network_function> pkt_counter_nf_ptr(new pkt_counter(pkt_counter_type));
    pkt_counter_nf_ptr->init();

    unique_ptr<network_function> firewall_nf_ptr(new firewall(firewall_type));
    firewall_nf_ptr->init();

    nf_map.emplace(make_pair(pkt_counter_type, std::move(pkt_counter_nf_ptr)));
    nf_map.emplace(make_pair(firewall_type, std::move(firewall_nf_ptr)));
  }

  network_function* get_nf_from_type_sig(uint8_t nf_type_sig){
    if(nf_map.find(nf_type_sig)==nf_map.end()){
      return nf_map[pkt_counter_type].get();
    }
    else{
      return nf_map[nf_type_sig].get();
    }
  }

  unique_ptr<flow_state> get_fs_from_type_sig(network_function* nf, vector<char>& buf){
    return nf->deserialize(sys, buf);
  }

private:
  actor_system& sys;
  map<uint8_t, unique_ptr<network_function>> nf_map;
};

#endif
