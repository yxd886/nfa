#ifndef RELIABLE_SEND_QUEUE_H
#define RELIABLE_SEND_QUEUE_H

#include "../../bessport/mem_alloc.h"
#include "../../bessport/packet.h"
#include "reliable_message_misc.h"
#include "../../actor/base/garbage_pkt_collector.h"

#include <glog/logging.h>
#include "../../rpc/ring_msg.h"

static constexpr bool is_power_of_two(uint32_t val){
  return (val!=0) &&
         ( (val==1) ||
           ( ((val&(0x00000001))==0) && is_power_of_two(val>>1) ) );
}

template<uint64_t N>
class reliable_send_queue{
public:
  static const uint64_t mask = N-1;

  static_assert(is_power_of_two(N), "N is not power of 2");

  reliable_send_queue(uint64_t local_rt_mac, uint64_t dest_rt_mac) :
    head_pos_(0), head_seq_num_(1),
    tail_pos_(0), next_seq_num_(1),
    cur_size_(0),
    window_pos_(0), window_pos_seq_num_(1),
    pending_send_num_(0){

    rh_.ethh.d_addr = *(reinterpret_cast<struct ether_addr*>(&dest_rt_mac));
    rh_.ethh.s_addr = *(reinterpret_cast<struct ether_addr*>(&local_rt_mac));
    rh_.ethh.ether_type = 0x0800;
    rh_.iph.version_ihl = 0x45;
    rh_.iph.fragment_offset = rte_cpu_to_be_16(IPV4_HDR_DF_FLAG);
    rh_.iph.time_to_live = 64;
    rh_.iph.next_proto_id = 0xFF;
    rh_.iph.src_addr = 0x0A0A0101;
    rh_.iph.dst_addr = 0x0A0A0102;
    rh_.magic_num = msg_magic_num;
  }

  void reset(garbage_pkt_collector* gp_collector){
    for(uint64_t i=0; i<cur_size_; i++){
      gp_collector->collect(ring_buf_[(head_pos_+i)&mask]);
    }

    head_pos_ = 0;
    head_seq_num_ = 1;

    tail_pos_ = 0;
    next_seq_num_ = 1;

    cur_size_ = 0;

    window_pos_ = 0;
    window_pos_seq_num_ = 1;

    pending_send_num_ = 0;
  }

  inline bool push(bess::Packet* obj_ptr){
    if(cur_size_==N){
      return false;
    }
    else{
      format_send_packet(obj_ptr);
      ring_buf_[tail_pos_] = obj_ptr;
      tail_pos_ = ((tail_pos_+1)&mask);
      cur_size_+=1;
      pending_send_num_+=1;
      return true;
    }
  }

  inline bool push(bess::PacketBatch* batch){
    if(unlikely(batch->cnt()+cur_size_>N)){
      return false;
    }

    for(int i=0; i<batch->cnt(); i++){
      push(batch->pkts()[i]);
    }
    return true;
  }

  inline bess::PacketBatch pop(uint32_t ack_seq_num){
    bess::PacketBatch batch;
    batch.clear();
    assert(batch.cnt()==0);

    if(unlikely(cur_size_ == 0)){
      return batch;
    }

    uint64_t pop_num = ack_seq_num - head_seq_num_;
    assert(pop_num<=cur_size_);


    if(unlikely(head_pos_+pop_num>=N)){
      batch.CopyAddr(ring_buf_+head_pos_, N-head_pos_);
      batch.CopyAddr(ring_buf_, pop_num-batch.cnt());
    }
    else{
      batch.CopyAddr(ring_buf_+head_pos_, pop_num);
    }

    head_pos_ = (head_pos_+pop_num)&mask;
    cur_size_ -= pop_num;
    head_seq_num_ = ack_seq_num;

    if(unlikely(ack_seq_num>window_pos_seq_num_)){
      window_pos_ = head_pos_;
      pending_send_num_ = cur_size_;
      window_pos_seq_num_ = head_seq_num_;
    }

    return batch;
  }

  inline bess::PacketBatch get_window_batch(uint64_t window_size){
    bess::PacketBatch batch;
    // bess::PacketBatch return_batch;
    batch.clear();
    // return_batch.clear();
    assert(batch.cnt()==0);

    if(unlikely(window_size>pending_send_num_)){
      window_size = pending_send_num_;
    }

    if(unlikely(window_pos_+window_size>=N)){
      for(uint64_t i=window_pos_; i<N; i++ ){
        batch.add(bess::Packet::copy(ring_buf_[i]));
      }
      for(uint64_t i=0; i<(window_size-N+window_pos_); i++){
        batch.add(bess::Packet::copy(ring_buf_[i]));
      }

      // batch.CopyAddr(ring_buf_+window_pos_, N-window_pos_);
      // batch.CopyAddr(ring_buf_, window_size - batch.cnt());
    }
    else{
      for(uint64_t i=window_pos_; i<(window_pos_+window_size); i++){
        batch.add(bess::Packet::copy(ring_buf_[i]));
      }
      // batch.CopyAddr(ring_buf_+window_pos_, window_size);
    }

    // for(int i=0; i<batch.cnt(); i++){
    //   return_batch.add(bess::Packet::copy(batch.pkts()[i]));
    // }

    window_pos_ = (window_pos_+window_size)&mask;
    pending_send_num_ -= window_size;
    window_pos_seq_num_ += window_size;

    return batch;
  }

private:
  inline int smaller(uint64_t first, uint64_t second){
    return (first>second)?second:first;
  }

  inline void format_send_packet(bess::Packet* pkt){
    int pkt_len = pkt->total_len()+
                  sizeof(struct ipv4_hdr)+
                  2*sizeof(uint32_t);

    rh_.iph.total_length = rte_cpu_to_be_16(pkt_len);
    rh_.iph.hdr_checksum = rte_ipv4_cksum(&(rh_.iph));
    rh_.seq_num = next_seq_num_;
    next_seq_num_ += 1;

    reliable_header* rh = reinterpret_cast<reliable_header*>(pkt->prepend(sizeof(reliable_header)));
    assert(rh!=nullptr);
    rte_memcpy(rh, &rh_, sizeof(reliable_header));
  }

  uint64_t head_pos_;
  uint32_t head_seq_num_;
  uint64_t tail_pos_;
  uint32_t next_seq_num_;
  uint64_t cur_size_;

  uint64_t window_pos_;
  uint32_t window_pos_seq_num_;
  uint64_t pending_send_num_;

  bess::Packet* ring_buf_[N];

  reliable_header rh_;
};

#endif
