#include "coordinator.h"
#include "flow_actor_allocator.h"
#include "../actor/base/local_send.h"

coordinator::coordinator(flow_actor_allocator* allocator){
  allocator_ = allocator;
  htable_.Init(flow_key_size, sizeof(flow_actor*));
  deadend_flow_actor_ = allocator_->allocate();
  nfa_ipv4_field::nfa_init_ipv4_field(fields_);
}

void coordinator::handle_message(es_scheduler_pkt_batch_t, bess::PacketBatch* batch){
  ec_scheduler_batch_.clear();
  char keys[bess::PacketBatch::kMaxBurst][flow_key_size] __ymm_aligned;

  for(int i=0; i<batch->cnt(); i++){
    char* data_start = reinterpret_cast<char *>(batch->pkts()[i]->buffer());
    data_start += batch->pkts()[i]->data_off();

    if(unlikely( ((*((uint16_t*)(data_start+14)) & 0x00f0) != 0x0040) ||
                 ( ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x0006) &&
                   ((*((uint16_t*)(data_start+23)) & 0x00ff) != 0x0011) ) ) ){
      int next_available_pos = ec_scheduler_batch_.cnt();
      es_scheduler_gates_[next_available_pos] = 1;
      ec_scheduler_batch_.add(batch->pkts()[i]);
      continue;
    }

    memset(&keys[i][flow_key_size-8], 0, sizeof(uint64_t));
    for(int j=0; j<3; j++){
      char* key = keys[i]+fields_[j].pos;
      *(uint64_t *)key = *(uint64_t *)(data_start + fields_[j].offset) & fields_[j].mask;
    }

    flow_actor** actor_ptr = htable_.Get(reinterpret_cast<flow_key_t*>(keys[i]));
    flow_actor* actor = 0;

    if(unlikely(actor_ptr==nullptr)){
      actor = allocator_->allocate();

      if(unlikely(actor==nullptr)){
        actor = deadend_flow_actor_;
      }
      else{
        send(actor, ec_scheduler_batch_and_gates_t::value, &ec_scheduler_batch_, es_scheduler_gates_,
            this);
      }

      htable_.Set(reinterpret_cast<flow_key_t*>(keys[i]), &actor);

      actor_ptr = &actor;
    }

    send(*actor_ptr, pkt_msg_t::value, batch->pkts()[i]);
  }
}
