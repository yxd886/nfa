#ifndef RELIABLE_P2P_H
#define RELIABLE_P2P_H

#include "./base/reliable_send_queue.h"
#include "../actor/base/local_message.h"
#include "./base/reliable_message_misc.h"
#include "../bessport/worker.h"

static constexpr size_t pkt_sub_msg_cutting_thresh = 1522-55-2;

class coordinator;

class reliable_p2p{
public:

  reliable_p2p(uint64_t local_rt_mac, uint64_t dest_rt_mac,
               int local_rtid, int dest_rtid, coordinator* coordinator_actor);

  reliable_single_msg* recv(bess::Packet* pkt);

  template<class T, uint16_t N>
  bool reliable_send(uint32_t msg_id,
                     uint32_t send_actor_id,
                     uint32_t recv_actor_id,
                     local_message_derived<N>,
                     T* cstruct_ptr){
    bess::Packet* cstruct_msg_pkt = create_cstruct_sub_msg(cstruct_ptr);
    if(unlikely(cstruct_msg_pkt == nullptr)){
      return false;
    }

    reliable_message_header* msg_header = reinterpret_cast<reliable_message_header*>(
                                          cstruct_msg_pkt->prepend(sizeof(reliable_message_header)));

    msg_header->send_actor_id = send_actor_id;
    msg_header->recv_actor_id = recv_actor_id;
    msg_header->msg_id = msg_id;
    msg_header->msg_type = N;
    msg_header->msg_pkt_num = 1;

    send_queue_.push(cstruct_msg_pkt);

    add_to_reliable_send_list(1);

    return true;
  }

  inline bess::Packet* get_ack_pkt(){
    bess::Packet* ack_pkt = bess::Packet::Alloc();
    ack_pkt->set_data_off(SNBUF_HEADROOM);
    ack_pkt->set_total_len(sizeof(reliable_header));
    ack_pkt->set_data_len(sizeof(reliable_header));

    ack_header_.seq_num = next_seq_num_to_recv_;

    char* data_start = ack_pkt->head_data<char*>();
    rte_memcpy(data_start, &ack_header_, sizeof(reliable_header));

    return ack_pkt;
  }

  void reset();

  inline void inc_ref_cnt(){
    ref_cnt_+=1;
  }

  inline void dec_ref_cnt(){
    ref_cnt_-=1;
  }

  inline bool is_ref_cnt_zero(){
    if(ref_cnt_==0){
      return true;
    }
    else{
      return false;
    }
  }

private:
  void add_to_reliable_send_list(int pkt_num);

  template<class T>
  bess::Packet* create_cstruct_sub_msg(T* cstruct_msg);

  void encode_binary_fs_sub_msg(bess::PacketBatch* batch);

  bess::PacketBatch create_packet_sub_msg(bess::Packet* pkt);

  reliable_send_queue<512> send_queue_;
  uint32_t next_seq_num_to_recv_;
  int ref_cnt_;

  int32_t local_rtid_;
  int32_t dest_rtid_;
  coordinator* coordinator_actor_;

  struct ether_addr local_runtime_mac_addr_;
  struct ether_addr dst_runtime_mac_addr_;

  bess::PacketBatch batch_;
  reliable_single_msg cur_msg_;

  reliable_header ack_header_;

  uint16_t output_gate_;
};

#endif
