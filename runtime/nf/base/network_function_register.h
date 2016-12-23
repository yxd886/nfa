#ifndef NETWORK_FUNCTION_REGISTER_H
#define NETWORK_FUNCTION_REGISTER_H

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include "network_function_derived.h"

class network_function_register{
private:
  static std::unordered_map<std::string, uint8_t>& name_id_map();
  static std::unordered_map<uint8_t, std::unique_ptr<network_function_base>>& id_nf_map();

  static uint8_t compute_network_function(uint64_t s, int pos);
  static int compute_service_chain_length(uint64_t s);

public:

  template<class NF, class NFS, class... NFArgs>
  static bool register_nf(std::string nf_name, uint8_t nf_id, NFArgs&&... args);

  static std::vector<network_function_base*> get_service_chain(uint64_t service_chain_type_sig);

  static void init(size_t nf_state_num);
};

#endif
