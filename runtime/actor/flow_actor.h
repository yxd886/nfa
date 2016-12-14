
#ifndef FLOW_ACTOR_H
#define FLOW_ACTOR_H

#include "actor.h"
#include "nf_ec_timer.h"
#include "network_function_hub.h"
#include "network_function.h"
#include "service_chain_state.h"
#include "concurrentqueue.h"


using std::vector;
using std::tuple;
using std::get;
using std::string;



class flow_actor :public actor{

public:

  bool servce_chain_process(struct rte_mbuf* input_pkt, bool from_p0);
  void process_pkt(struct rte_mbuf* input_pkt, bool from_p0);

  int internal_pkt_counter;
	network_function_hub& hub;
  int local_rt_id;
  vector<char> flow_identifier;
  actor worker_a;

  // the service chain type
  service_chain_t service_chain_type_sig;

  vector<tuple<network_function*, unique_ptr<flow_state>>> service_chain;

  moodycamel::ConcurrentQueue<struct rte_mbuf*>* output_queue;


};
#endif
