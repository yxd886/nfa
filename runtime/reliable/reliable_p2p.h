#ifndef RELIABLE_P2P_H
#define RELIABLE_P2P_H

#include "./base/reliable_send_queue.h"

class reliable_p2p{
public:
  int get();

  /*inline void recv(bess::PacketBatch* batch){
    for(int i=0; i<batch->cnt(); i++){
      char* data_start = reinterpret_cast<char *>(batch->pkts()[i]->buffer());
      data_start += batch->pkts()[i]->data_off();

      uint32_t* magic_num_ptr = reinterpret_cast<uint32_t*>(data_start+
                                                            sizeof(struct ether_hdr) +
                                                            sizeof(struct ipv4_hdr));

      if(*magic_num_ptr == 0x23456789){
        // this is an ack sent from the remote runtime.
        uint32_t ack_seq_num = *(magic_num_ptr+1);
        bess::PacketBatch pop_batch = send_queue_.pop(ack_seq_num - head_seq_num_);
        bess::Packet::Free(&pop_batch);
        head_seq_num_ = ack_seq_num;
      }
      else{
        // this is a msg sent from the remote runtime
        uint32_t msg_seq_num = *(magic_num_ptr+1);
        if(msg_seq_num == next_seq_num_to_recv_){
          next_seq_num_to_recv_ += 1;
        }
        recv_batch_.add(batch->pkts()[i]);
      }
    }
    bess::Packet::Free(&recv_batch_);
    recv_batch_.clear();
  }*/

private:
  struct ether_addr local_runtime_mac_addr_;
  struct ether_addr dst_runtime_mac_addr_;

  uint64_t send_window_pos_;
  uint32_t send_seq_num_;
  uint32_t head_seq_num_;
  reliable_send_queue<64, bess::Packet> send_queue_;

  uint32_t next_seq_num_to_recv_;
  bess::PacketBatch recv_batch_;

};

#endif
