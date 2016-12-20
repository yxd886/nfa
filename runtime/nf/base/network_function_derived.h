#ifndef NETWORK_FUNCTION_DERIVED_H
#define NETWORK_FUNCTION_DERIVED_H

#include "network_function_base.h"

template<class TNF, class TNFState, class... TNFArgs>
class network_function_derived : public network_function_base{
public:
  network_function_derived(size_t nf_state_num, TNFArgs&&... tnf_args) :
    network_function_base(nf_state_num, sizeof(TNFState)),
    nf_instance_(std::forward<TNFArgs>(tnf_args)...){
    static_assert(std::is_pod<TNFState>::value, "NF flow state is not a POD Type");
  }

  inline void nf_logic(bess::Packet* pkt, char* state_ptr) override{
    nf_instance_.nf_logic_impl(pkt, reinterpret_cast<TNFState*>(state_ptr));
  }

  ~network_function_derived() = default;

private:
  TNF nf_instance_;
};

#endif
