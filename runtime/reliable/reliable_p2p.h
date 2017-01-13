#ifndef RELIABLE_P2P_H
#define RELIABLE_P2P_H

#include "./base/reliable_send_queue.h"
#include "../actor/base/local_message.h"
#include "./base/reliable_message_misc.h"
#include "../bessport/worker.h"

static constexpr size_t pkt_sub_msg_cutting_thresh = 1522-55-2;

static constexpr int initial_check_times = 50;

static constexpr int next_check_times = 50;

class coordinator;

class reliable_p2p{
public:

  reliable_p2p(uint64_t local_rt_mac, uint64_t dest_rt_mac,
               int local_rtid, int dest_rtid, coordinator* coordinator_actor,
               uint16_t output_gate, runtime_config* remote_rt_config);

  reliable_single_msg* recv(bess::Packet* pkt);

  template<class T, uint16_t N>
  bool reliable_send(uint32_t msg_id,
                     uint32_t send_actor_id,
                     uint32_t recv_actor_id,
                     local_message_derived<N>,
                     T* cstruct_ptr){
    if(is_connection_up_ == false){
      return false;
    }

    bess::Packet* cstruct_msg_pkt = create_cstruct_sub_msg(cstruct_ptr);
    if(unlikely(cstruct_msg_pkt == nullptr)){
      // assert(1==0);
      return false;
    }

    reliable_message_header* msg_header = reinterpret_cast<reliable_message_header*>(
                                          cstruct_msg_pkt->prepend(sizeof(reliable_message_header)));

    msg_header->send_actor_id = send_actor_id;
    msg_header->recv_actor_id = recv_actor_id;
    msg_header->msg_id = msg_id;
    msg_header->msg_type = N;
    msg_header->msg_pkt_num = 1;

    bool flag = send_queue_.push(cstruct_msg_pkt);
    if(unlikely(flag == false)){
      bess::Packet::Free(cstruct_msg_pkt);
      // assert(1==0);
      return false;
    }

    add_to_reliable_send_list(1);

    return true;
  }

  template<uint16_t N>
  bool reliable_send(uint32_t msg_id,
                     uint32_t send_actor_id,
                     uint32_t recv_actor_id,
                     local_message_derived<N>,
                     bess::PacketBatch* batch){
    if(is_connection_up_ == false){
      return false;
    }

    encode_binary_fs_sub_msg(batch);

    reliable_message_header* msg_header = reinterpret_cast<reliable_message_header*>(
                                          batch->pkts()[0]->prepend(sizeof(reliable_message_header)));

    msg_header->send_actor_id = send_actor_id;
    msg_header->recv_actor_id = recv_actor_id;
    msg_header->msg_id = msg_id;
    msg_header->msg_type = N;
    msg_header->msg_pkt_num = batch->cnt();


    bool flag = send_queue_.push(batch);
    if(unlikely(flag == false)){
      // assert(1==0);
      bess::Packet::Free(batch);
      return false;
    }

    add_to_reliable_send_list(batch->cnt());

    return true;
  }

  inline bess::PacketBatch get_send_batch(int batch_size){
    return send_queue_.get_window_batch(batch_size);
  }

  inline bess::Packet* get_ack_pkt(){
    if(next_seq_num_to_recv_snapshot_ == next_seq_num_to_recv_){
      return nullptr;
    }

    bess::Packet* ack_pkt = bess::Packet::Alloc();
    if(ack_pkt == nullptr){
      return nullptr;
    }

    ack_pkt->set_data_off(SNBUF_HEADROOM);
    ack_pkt->set_total_len(sizeof(reliable_header));
    ack_pkt->set_data_len(sizeof(reliable_header));

    ack_header_.seq_num = next_seq_num_to_recv_;
    next_seq_num_to_recv_snapshot_ = next_seq_num_to_recv_;

    char* data_start = ack_pkt->head_data<char*>();
    rte_memcpy(data_start, &ack_header_, sizeof(reliable_header));
    return ack_pkt;
  }

  void check(uint64_t current_ns);

  inline uint64_t peek_rtt(){
    return send_queue_.peek_rtt();
  }

  inline uint16_t get_output_gate(){
    return output_gate_;
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

  inline runtime_config* get_rt_config(){
    return &remote_rt_config_;
  }

  inline bool check_connection_status(){
    return is_connection_up_;
  }

private:
  void add_to_reliable_send_list(int pkt_num);
  void prepend_to_reliable_send_list(int pkt_num);

  template<class T>
  bess::Packet* create_cstruct_sub_msg(T* cstruct_msg){
    static_assert(std::is_pod<T>::value, "The type of cstruct_msg is not POD");
    static_assert(sizeof(T)<pkt_sub_msg_cutting_thresh,
                  "The size of cstruct_msg is too large to fit into a single packet");

    bess::Packet* msg_pkt = bess::Packet::Alloc();
    if(msg_pkt == nullptr){
      return nullptr;
    }

    msg_pkt->set_data_off(SNBUF_HEADROOM);
    msg_pkt->set_total_len(sizeof(T)+sizeof(uint8_t));
    msg_pkt->set_data_len(sizeof(T)+sizeof(uint8_t));

    char* sub_msg_tag =  reinterpret_cast<char *>(msg_pkt->buffer()) +
                         static_cast<size_t>(SNBUF_HEADROOM);
    *sub_msg_tag = static_cast<char>(sub_message_type_enum::cstruct);

    char* cstruct_msg_start = sub_msg_tag+1;
    rte_memcpy(cstruct_msg_start, cstruct_msg, sizeof(T));

    return msg_pkt;
  }

  inline void encode_binary_fs_sub_msg(bess::PacketBatch* batch){
    // the batch contains binary flow states created by the network functions.
    assert(batch->cnt()>0);

    uint8_t* sub_msg_num = reinterpret_cast<uint8_t*>(batch->pkts()[0]->prepend(1));
    *sub_msg_num = batch->cnt();

    char* sub_msg_tag = reinterpret_cast<char*>(batch->pkts()[0]->prepend(1));
    *sub_msg_tag =  static_cast<char>(sub_message_type_enum::packet);
  }

  inline bess::PacketBatch create_packet_sub_msg(bess::Packet* pkt){
    bess::PacketBatch batch;

    batch.add(pkt);
    char* pkt_data_start = reinterpret_cast<char *>(pkt->buffer()) + pkt->data_off();

    if(unlikely(pkt->data_len()>pkt_sub_msg_cutting_thresh)){
      bess::Packet* suplement_pkt = bess::Packet::Alloc();

      if(unlikely(suplement_pkt == nullptr)){
        batch.clear();
        return batch;
      }

      size_t suplement_pkt_size = pkt->data_len() - pkt_sub_msg_cutting_thresh;
      suplement_pkt->set_data_off(SNBUF_HEADROOM);
      suplement_pkt->set_total_len(suplement_pkt_size);
      suplement_pkt->set_data_len(suplement_pkt_size);

      char* suplement_pkt_data_start = reinterpret_cast<char *>(suplement_pkt->buffer()) +
                                       static_cast<size_t>(SNBUF_HEADROOM);
      rte_memcpy(suplement_pkt_data_start, pkt_data_start+pkt_sub_msg_cutting_thresh, suplement_pkt_size);

      batch.add(suplement_pkt);
    }

    uint8_t* sub_msg_num = reinterpret_cast<uint8_t*>(pkt->prepend(1));
    *sub_msg_num = batch.cnt();

    char* sub_msg_tag = reinterpret_cast<char*>(pkt->prepend(1));
    *sub_msg_tag =  static_cast<char>(sub_message_type_enum::packet);

    return batch;
  }

  reliable_send_queue<reliable_send_queue_size> send_queue_;
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

  uint32_t next_seq_num_to_recv_snapshot_;

  uint64_t next_check_time_;
  uint64_t last_check_head_seq_num_;
  uint64_t consecutive_counter_;

  runtime_config remote_rt_config_;

  bool is_connection_up_;
};

#endif
