//
#include "ec_scheduler.h"
#include <glog/logging.h>

struct ipv4_field{
  uint16_t pos;
  uint16_t offset;
  uint32_t size;
  uint64_t mask;
};

static ipv4_field fields[3];

static void init_ipv4_field(ipv4_field* f){
  // protocol
  f[0].pos = 0;
  f[0].offset = 23;
  f[0].size = 1;
  f[0].mask = ((uint64_t)1 << (f[0].size * 8)) - 1;

  // src/dst ip
  f[1].pos = 1;
  f[1].offset = 26;
  f[1].size = 8;
  f[1].mask = 0xffffffffffffffff;

  // src/dst port
  f[2].pos = 9;
  f[2].offset = 34;
  f[2].size = 4;
  f[2].mask = ((uint64_t)1 << (f[2].size * 8)) - 1;
}

pb_error_t ec_scheduler::Init(const bess::pb::PortIncArg &arg){
  return pb_errno(0);
}

void ec_scheduler::ProcessBatch(bess::PacketBatch *batch){
  LOG(INFO)<<"in ProcessBatch with batch size "<<batch->cnt();

  char keys[bess::PacketBatch::kMaxBurst][flow_key_size] __ymm_aligned;
  init_ipv4_field(fields);

  for(int i=0; i<batch->cnt(); i++){
    LOG(INFO)<<batch->pkts()[i]->Dump();

    char* data_start = reinterpret_cast<char *>(batch->pkts()[i]->buffer());
    data_start += batch->pkts()[i]->data_off();

    memset(&keys[i][flow_key_size-8], 0, sizeof(uint64_t));
    for(int j=0; j<3; j++){
      char* key = keys[i]+fields[j].pos;
      *(uint64_t *)key = *(uint64_t *)(data_start + fields[j].offset) & fields[j].mask;
    }

    LOG(INFO)<<"before ht Get";
    flow_actor* actor = *(static_cast<flow_actor**>(htable_.Get(reinterpret_cast<flow_key_t*>(keys[i]))));
    LOG(INFO)<<"after ht Get";


    if(unlikely(actor==nullptr)){
      actor = allocator_->allocate();

      if(unlikely(actor==nullptr)){
        actor = deadend_flow_actor_;
      }

      htable_.Set(reinterpret_cast<flow_key_t*>(keys[i]), &actor);
    }

    send(actor, pkt_msg_t::value, batch->pkts()[i]);
  }

  RunNextModule(batch);
}

void ec_scheduler::customized_init(flow_actor_allocator* allocator){
  allocator_ = allocator;
  deadend_flow_actor_ = allocator_->allocate();
  htable_.Init(flow_key_size, sizeof(flow_actor*));
}

ADD_MODULE(ec_scheduler, "ec_scheduler", "schedule flow actors")
