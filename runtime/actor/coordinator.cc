#include "coordinator.h"
#include "flow_actor_allocator.h"
#include "../actor/base/local_send.h"
#include "../nf/pktcounter/pkt_counter.h"
#include "../nf/base/network_function_derived.h"
#include "../nf/base/nf_item.h"

#include <glog/logging.h>

coordinator::coordinator(flow_actor_allocator* allocator){
  allocator_ = allocator;
  htable_.Init(flow_key_size, sizeof(flow_actor*));
  deadend_flow_actor_ = allocator_->allocate();
  nfa_ipv4_field::nfa_init_ipv4_field(fields_);

  service_chain_.push_back(new network_function_derived<pkt_counter, pkt_counter_fs>(allocator_->get_max_actor()));
}

void coordinator::handle_message(es_scheduler_pkt_batch_t, bess::PacketBatch* batch){
  ec_scheduler_batch_.clear();
  char keys[bess::PacketBatch::kMaxBurst][flow_key_size] __ymm_aligned;
  nf_item nf_items[8];

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
        LOG(WARNING)<<"No available flow actors to allocate";
        actor = deadend_flow_actor_;
      }
      else{
        bool fs_allocation_failure = false;

        for(size_t i=0; i<service_chain_.size(); i++){
          char* fs_ptr = service_chain_[i]->allocate();
          if(unlikely(fs_ptr==nullptr)){
            fs_allocation_failure = true;
            break;
          }
          else{
            nf_items[i].nf = service_chain_[i];
            nf_items[i].nf_flow_state_ptr = fs_ptr;
            nf_items[i].nf_flow_state_size = service_chain_[i]->get_nf_state_size();
          }
        }

        if(unlikely(fs_allocation_failure == true)){
          allocator_->deallocate(actor);
          actor = deadend_flow_actor_;
          LOG(WARNING)<<"No available flow states to allocate";
        }
        else{
          send(actor, flow_actor_init_t::value,
                       this, reinterpret_cast<flow_key_t*>(keys[i]),
                       nf_items, service_chain_.size());
        }
      }

      htable_.Set(reinterpret_cast<flow_key_t*>(keys[i]), &actor);

      actor_ptr = &actor;
    }

    send(*actor_ptr, pkt_msg_t::value, batch->pkts()[i]);
  }
}

void coordinator::handle_message(remove_flow_t, flow_actor* flow_actor){

  htable_.Del(flow_actor->peek_flow_key());

  if(flow_actor!=deadend_flow_actor_){
    for(size_t i=0; i<flow_actor->peek_service_chain_length(); i++){
      nf_item* item = &(flow_actor->peek_nf_items()[i]);
      item->nf->deallocate(item->nf_flow_state_ptr);
    }
    allocator_->deallocate(flow_actor);
  }
  else{
  }
}
