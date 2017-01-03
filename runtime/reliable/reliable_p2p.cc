#include "reliable_p2p.h"
#include "../actor/coordinator.h"

reliable_p2p::reliable_p2p(uint64_t local_rt_mac, uint64_t dest_rt_mac,
                           int local_rtid, int dest_rtid, coordinator* coordinator_actor) :
  send_queue_(local_rt_mac, dest_rt_mac), next_seq_num_to_recv_(1), ref_cnt_(0),
  local_rtid_(local_rtid), dest_rtid_(dest_rtid), coordinator_actor_(coordinator_actor){
  local_runtime_mac_addr_ = *(reinterpret_cast<struct ether_addr*>(&local_rt_mac));
  dst_runtime_mac_addr_ = *(reinterpret_cast<struct ether_addr*>(&dest_rt_mac));
  cur_msg_.send_runtime_id = dest_rtid;

  ack_header_.ethh.d_addr = dst_runtime_mac_addr_;
  ack_header_.ethh.s_addr = local_runtime_mac_addr_;
  ack_header_.ethh.ether_type = 0x0800;

  ack_header_.iph.version_ihl = 0x45;
  ack_header_.iph.total_length = rte_cpu_to_be_16(sizeof(struct ipv4_hdr)+2);
  ack_header_.iph.fragment_offset = rte_cpu_to_be_16(IPV4_HDR_DF_FLAG);
  ack_header_.iph.time_to_live = 64;
  ack_header_.iph.next_proto_id = 0xFF;
  ack_header_.iph.src_addr = 0x0A0A0102;
  ack_header_.iph.dst_addr = 0x0A0A0101;
  ack_header_.iph.hdr_checksum = rte_ipv4_cksum(&(ack_header_.iph));

  ack_header_.magic_num = ack_magic_num;

  output_gate_ = 0;

  next_seq_num_to_recv_snapshot_ = 1;
}

reliable_single_msg* reliable_p2p::recv(bess::Packet* pkt){
  reliable_header* rh = pkt->head_data<reliable_header *>();

  if(unlikely(rh->magic_num == ack_magic_num)){
    bess::PacketBatch free_batch = send_queue_.pop(rh->seq_num);
    coordinator_actor_->gp_collector_.collect(&free_batch);
    coordinator_actor_->gp_collector_.collect(pkt);
    return nullptr;
  }

  if(unlikely(rh->seq_num != next_seq_num_to_recv_)){
    coordinator_actor_->gp_collector_.collect(pkt);
    return nullptr;
  }

  next_seq_num_to_recv_ += 1;
  if(batch_.cnt()==0){
    reliable_message_header* rmh = reinterpret_cast<reliable_message_header*>(rh+1);
    rte_memcpy(&(cur_msg_.rmh), rmh, sizeof(reliable_message_header));
    pkt->adj(sizeof(reliable_header)+sizeof(reliable_message_header));
    batch_.add(pkt);
  }
  else{
    pkt->adj(sizeof(reliable_header));
    batch_.add(pkt);
  }

  if(batch_.cnt() == cur_msg_.rmh.msg_pkt_num){
    cur_msg_.format(&batch_);
    batch_.clear();
    return &cur_msg_;
  }
  else{
    return nullptr;
  }
}

void reliable_p2p::reset(){
  send_queue_.reset(&(coordinator_actor_->gp_collector_));
  next_seq_num_to_recv_ = 1;
  cur_msg_.clean(&(coordinator_actor_->gp_collector_));
  batch_.clear();
}

void reliable_p2p::add_to_reliable_send_list(int pkt_num){
  generic_list_item* last_item = coordinator_actor_->reliable_send_list_.peek_tail();

  if(unlikely(last_item->reliable_rtid != local_rtid_)){
    generic_list_item* list_item = coordinator_actor_->get_list_item_allocator()->allocate();

    list_item->pkt_num = pkt_num;
    list_item->reliable_rtid = local_rtid_;
    list_item->output_gate = output_gate_;

    coordinator_actor_->reliable_send_list_.add_to_tail(list_item);

    return;
  }

  last_item->pkt_num += pkt_num;
}

template<class T>
bess::Packet* reliable_p2p::create_cstruct_sub_msg(T* cstruct_msg){
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

void reliable_p2p::encode_binary_fs_sub_msg(bess::PacketBatch* batch){
  // the batch contains binary flow states created by the network functions.
  assert(batch->cnt()>0);

  uint8_t* sub_msg_num = reinterpret_cast<uint8_t*>(batch->pkts()[0]->prepend(1));
  *sub_msg_num = batch->cnt();

  char* sub_msg_tag = reinterpret_cast<char*>(batch->pkts()[0]->prepend(1));
  *sub_msg_tag =  static_cast<char>(sub_message_type_enum::packet);
}

bess::PacketBatch reliable_p2p::create_packet_sub_msg(bess::Packet* pkt){
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
