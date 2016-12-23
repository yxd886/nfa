#include <cassert>

#include <glog/logging.h>

#include "network_function_register.h"

std::unordered_map<std::string, uint8_t>& network_function_register::name_id_map(){
  static std::unordered_map<std::string, uint8_t> name_id_map;
  return name_id_map;
}

std::unordered_map<uint8_t, std::unique_ptr<network_function_base>>&
network_function_register::id_nf_map(){
  static std::unordered_map<uint8_t, std::unique_ptr<network_function_base>> id_nf_map;
  return id_nf_map;
}

uint8_t network_function_register::compute_network_function(uint64_t s, int pos){
  return static_cast<uint8_t>((s>>(8*pos))&0x00000000000000FF);
}

int network_function_register::compute_service_chain_length(uint64_t s){
  int length = 0;
  bool encounter_zero = false;
  for(int i=0; i<8; i++){
    uint8_t nf =
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


template<class NF, class NFS, class... NFArgs>
bool network_function_register::register_nf(std::string nf_name, uint8_t nf_id, NFArgs&&... args){
  if( (name_id_map().find(nf_name) != name_id_map().end()) ||
      (id_nf_map().find(nf_id) != id_nf_map().end()) ){
    LOG(ERROR)<<"nf_name : "<<nf_name<<" or nf_id : "<<nf_id<<" has been used";
    return false;
  }
  name_id_map().emplace(nf_name, nf_id);

  std::unique_ptr<network_function_base> ptr(new network_function_derived<NF, NFS>(nf_id,
                                                                                   std::forward<NFArgs>(args)...));

  id_nf_map().emplace(nf_id, std::move(ptr));
  return true;
}

std::vector<network_function_base*>
network_function_register::get_service_chain(uint64_t service_chain_type_sig){
  std::vector<network_function_base*> v;
  for(int i=0; i<compute_service_chain_length(service_chain_type_sig); i++){
    uint8_t nf_id = compute_network_function(service_chain_type_sig, i);
    v.push_back(id_nf_map().find(nf_id)->second.get());
  }
  return v;
}

void network_function_register::init(size_t nf_state_num){
  for(auto it = id_nf_map().begin(); it!=id_nf_map().end(); it++){
    it->second->init_ring(nf_state_num);
  }
}
