#include "reliable_sender.h"
#include "../bessport/metadata.h"

struct task_result reliable_sender::RunTask(void *arg) {
  const uint8_t qid = 0;
  bess::PacketBatch batch;
  struct task_result ret;

  uint64_t received_bytes = 0;
  const int burst = bess::PacketBatch::kMaxBurst;
  const int pkt_overhead = 24;

  uint64_t cnt;
  batch.set_cnt(port_->RecvPackets(qid, batch.pkts(), burst));
  cnt = batch.cnt();

  if (cnt == 0) {
    ret.packets = 0;
    ret.bits = 0;
    return ret;
  }

  for (uint64_t i = 0; i < cnt; i++)
    received_bytes += batch.pkts()[i]->total_len();


  ret = (struct task_result){
      .packets = cnt, .bits = (received_bytes + cnt * pkt_overhead) * 8,
  };

  port_->queue_stats[PACKET_DIR_INC][qid].packets += cnt;
  port_->queue_stats[PACKET_DIR_INC][qid].bytes += received_bytes;

  RunNextModule(&batch);

  return ret;
}

void reliable_sender::ProcessBatch(bess::PacketBatch *batch) {
  LOG(INFO)<<"reliable_sender receives "<<batch->cnt()<<" packets.";
  bess::Packet::Free(batch);
}

void reliable_sender::customized_init(sn_port* port){
  RegisterTask(nullptr);
  port_ = port;
}

ADD_MODULE(reliable_sender, "reliable_sender", "reliably send packets")
