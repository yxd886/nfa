#ifndef RELIABLE_MESSAGE_MISC_H
#define RELIABLE_MESSAGE_MISC_H

#include <cstdint>

#include <rte_config.h>
#include <rte_ether.h>
#include <rte_ip.h>

#include <iostream>

#include "../../bessport/packet.h"
#include "../../actor/base/garbage_pkt_collector.h"

static constexpr int reliable_send_queue_size = 4096;

enum class sub_message_type_enum : char{
  cstruct,
  binary_flow_state,
  packet
};

struct reliable_header{
  struct ether_hdr ethh;
  struct ipv4_hdr iph;
  uint8_t magic_num;
  uint32_t seq_num;
};

struct reliable_message_header {
  uint32_t send_actor_id;
  uint32_t recv_actor_id;
  uint32_t msg_id;
  uint16_t msg_type;
  uint16_t msg_pkt_num;
};

/*
 * uint8_t
 * */

static constexpr uint8_t msg_magic_num = 0xA1;

static constexpr uint8_t ack_magic_num = 0xB2;

struct reliable_single_msg{
  int32_t send_runtime_id;

  reliable_message_header rmh;

  bess::Packet* cstruct_pkt;

  bess::PacketBatch fs_msg_batch;

  bess::Packet* raw_pkt;

  bess::PacketBatch garbage;

  inline void init(){
    fs_msg_batch.clear();
    garbage.clear();
  }

  inline bool format(bess::PacketBatch* batch){
    int i = 0;
    bool return_flag = true;
    while(i<batch->cnt()){
      char* sub_msg_tag = batch->pkts()[i]->head_data<char*>();
      switch(*sub_msg_tag){
        case static_cast<char>(sub_message_type_enum::cstruct) : {
          cstruct_pkt = batch->pkts()[i];
          cstruct_pkt->adj(1);
          garbage.add(cstruct_pkt);
          i+=1;
          break;
        }
        case static_cast<char>(sub_message_type_enum::binary_flow_state) : {
          uint8_t num = (*reinterpret_cast<uint8_t*>(sub_msg_tag+1));
          batch->pkts()[i]->adj(2);
          for(uint32_t j=0; j<num; j++){
            fs_msg_batch.add(batch->pkts()[i+j]);
            garbage.add(batch->pkts()[i+j]);
          }
          i+=num;
          break;
        }
        case static_cast<char>(sub_message_type_enum::packet) : {
          uint8_t num = (*reinterpret_cast<uint8_t*>(sub_msg_tag+1));
          batch->pkts()[i]->adj(2);
          if(unlikely(num == 2)){
            uint16_t copy_size = batch->pkts()[i+1]->data_len();
            char* copy_start = reinterpret_cast<char*>(batch->pkts()[i]->append(copy_size));
            rte_memcpy(copy_start, batch->pkts()[i]->head_data<char*>(), copy_size);
            garbage.add(batch->pkts()[i+1]);
          }
          raw_pkt = batch->pkts()[i];
          i+=num;
          break;
        }
        default:{
          LOG(INFO)<<"Fatal error, we should enter here";
          std::cout<<"!!!!!The message tag is "<<std::hex<<*sub_msg_tag<<std::endl;
          LOG(INFO)<<"The current i is "<<i;
          for(int j=0; j<batch->cnt(); j++){
            LOG(INFO)<<batch->pkts()[j]->Dump();
          }
          return_flag = false;
          i = batch->cnt();
          break;
        }
      }
    }

    return return_flag;
  }

  inline void clean(garbage_pkt_collector* gp_collector){
    gp_collector->collect(&garbage);
    fs_msg_batch.clear();
    garbage.clear();
  }
};

#endif
