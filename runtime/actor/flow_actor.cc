#include "flow_actor.h"
#include <iostream>
using std::cout;
using std::endl;

bool flow_actor::servce_chain_process(struct rte_mbuf* input_pkt, bool from_p0){
  if(from_p0){
    internal_pkt_counter+=1;
    // do a forward service chain process
    bool pass = true;
    for(size_t i=0; i<service_chain.size(); i++){
      auto fs = get<1>(service_chain[i]).get();
      pass = get<0>(service_chain[i])->process_pkt(input_pkt, fs);

      // if(pass == false){
      //   return pass;
      // }
    }
    return pass;
  }
  else{
    bool pass = true;
    for(int i=service_chain.size()-1; i>=0; i--){
      auto fs = get<1>(service_chain[i]).get();
      pass = get<0>(service_chain[i])->process_pkt(input_pkt, fs);

      // if(pass == false){
      //   return pass;
      // }
    }
    return pass;
  }
}



void flow_actor::process_pkt(struct rte_mbuf* input_pkt, bool from_p0){

    // process
    servce_chain_process(input_pkt, from_p0);

    // send to the output queue
    while(!output_queue->try_enqueue(input_pkt)){
    }



}



