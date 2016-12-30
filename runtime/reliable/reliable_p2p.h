#ifndef RELIABLE_P2P_H
#define RELIABLE_P2P_H

#include "./base/reliable_send_queue.h"

class reliable_p2p{
public:
  int get();

  reliable_p2p(uint64_t local_rt_mac, uint64_t dest_rt_mac) {}

  inline void recv(bess::PacketBatch* batch){
    for(int i=0; i<batch->cnt(); i++){
      char* data_start = reinterpret_cast<char *>(batch->pkts()[i]->buffer()) +
                         batch->pkts()[i]->data_off();

      uint32_t* magic_num_ptr = reinterpret_cast<uint32_t*>(data_start+
                                                               sizeof(struct ether_hdr) +
                                                               sizeof(struct ipv4_hdr));

      if(((*magic_num_ptr)&0x00000003) == 2){
        // this is an ack sent from the remote runtime.
        uint32_t ack_seq_num = *(magic_num_ptr+1);
        send_queue_.pop(ack_seq_num);
      }
      else{
        // this is a msg sent from the remote runtime
        uint32_t msg_seq_num = *(magic_num_ptr+1);

        if(msg_seq_num != next_seq_num_to_recv_){
          continue;
        }

        next_seq_num_to_recv_ += 1;
      }
    }
  }

  inline void reset(){

  }

private:
  struct ether_addr local_runtime_mac_addr_;
  struct ether_addr dst_runtime_mac_addr_;

  reliable_send_queue<64, bess::Packet> send_queue_;

  uint32_t next_seq_num_to_recv_;
};

#endif
