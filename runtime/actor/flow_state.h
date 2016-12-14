#ifndef SRC_ACTOR_FLOW_STATE_HPP_
#define SRC_ACTOR_FLOW_STATE_HPP_

#include <vector>
#include <map>

using std::vector;
using std::map;

class flow_state{
public:
  explicit flow_state(uint8_t nf_type_sig) : nf_type_sig(nf_type_sig){};
  virtual ~flow_state() = default;
  uint8_t nf_type_sig;
};

#endif
