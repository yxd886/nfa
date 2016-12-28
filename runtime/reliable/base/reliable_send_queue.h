#ifndef RELIABLE_SEND_QUEUE_H
#define RELIABLE_SEND_QUEUE_H

#include "../../bessport/mem_alloc.h"
#include "../../bessport/packet.h"

#include <rte_udp.h>
#include <rte_ether.h>
#include <rte_ip.h>

static constexpr bool is_power_of_two(uint32_t val){
  return (val!=0) &&
         ( (val==1) ||
           ( ((val&(0x00000001))==0) && is_power_of_two(val>>1) ) );
}

template<uint64_t N, class T>
class reliable_send_queue{
};

template<uint64_t N>
class reliable_send_queue<N, bess::Packet>{
public:
  static const uint64_t mask = N-1;

  static_assert(is_power_of_two(N), "N is not power of 2");

  reliable_send_queue() :
    head_pos_(0), head_seq_num_(1),
    tail_pos_(0), next_seq_num_(1),
    cur_size_(0),
    window_pos_(0), window_pos_seq_num_(1),
    pending_send_num_(0){}

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
    assert(batch.cnt()==0);

    if(unlikely(window_size>pending_send_num_)){
      window_size = pending_send_num_;
    }

    if(unlikely(window_pos_+window_size>=N)){
      batch.CopyAddr(ring_buf_+window_pos_, N-window_pos_);
      batch.CopyAddr(ring_buf_, window_size - batch.cnt());
    }
    else{
      batch.CopyAddr(ring_buf_+window_pos_, window_size);
    }

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

    struct ether_hdr *ethh;
    struct ipv4_hdr *iph;
    uint32_t* magic_num_ptr;
    uint32_t* seq_num_ptr;

    ethh = static_cast<struct ether_hdr*>(
                   pkt->append( sizeof(struct ether_hdr) + // ethernet header
                                sizeof(struct ipv4_hdr) +  // ipv4 header
                                sizeof(uint32_t) +         // the magic number for raw msg
                                sizeof(uint32_t) ) );      // the msg sequential number

    iph = reinterpret_cast<struct ipv4_hdr*>(ethh+1);
    magic_num_ptr = reinterpret_cast<uint32_t*>(iph+1);
    seq_num_ptr = magic_num_ptr+1;

    *seq_num_ptr = next_seq_num_;
    next_seq_num_ += 1;

    *magic_num_ptr = 0x12340001;

    iph->version_ihl = 0x45;
    iph->total_length = rte_cpu_to_be_16(pkt_len);
    iph->fragment_offset = rte_cpu_to_be_16(IPV4_HDR_DF_FLAG);
    iph->time_to_live = 64;
    iph->next_proto_id = 0xFF;
    iph->src_addr = 0x0A0A0101;
    iph->dst_addr = 0x0A0A0102;
    iph->hdr_checksum = rte_ipv4_cksum(iph);

    ethh->d_addr = dst_runtime_mac_addr_;
    ethh->s_addr = local_runtime_mac_addr_;
    ethh->ether_type = 0x0800;
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

  struct ether_addr local_runtime_mac_addr_;
  struct ether_addr dst_runtime_mac_addr_;
};

#endif
