#ifndef SERVICE_CHAIN_STATE
#define SERVICE_CHAIN_STATE

//#include "caf/all.hpp"
#include <vector>

//using namespace caf;
using std::vector;

struct service_chain_state{
  vector<vector<char>> states;
};

template <class Inspector>
static typename Inspector::result_type inspect(Inspector& f, service_chain_state& x) {
  return f(meta::type_name("q"), x.states);
}

#endif
