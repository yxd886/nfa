#include "port_inc.h"
#include "../bessport/metadata.h"

struct task_result PortInc::RunTask(void *arg) {
  const uint8_t qid = 0;
  bess::PacketBatch batch;
  struct task_result ret;

  uint64_t received_bytes = 0;
  const int burst = ACCESS_ONCE(burst_);
  const int pkt_overhead = 24;

  uint64_t cnt;
  batch.set_cnt(port_->RecvPackets(qid, batch.pkts(), burst));
  cnt = batch.cnt();

  if (cnt == 0) {
    ret.packets = 0;
    ret.bits = 0;
    return ret;
  }

  if (prefetch_) {
    for (uint64_t i = 0; i < cnt; i++) {
      received_bytes += batch.pkts()[i]->total_len();
      rte_prefetch0(batch.pkts()[i]->head_data());
    }
  } else {
    for (uint64_t i = 0; i < cnt; i++)
      received_bytes += batch.pkts()[i]->total_len();
  }

  ret = (struct task_result){
      .packets = cnt, .bits = (received_bytes + cnt * pkt_overhead) * 8,
  };

  port_->queue_stats[PACKET_DIR_INC][qid].packets += cnt;
  port_->queue_stats[PACKET_DIR_INC][qid].bytes += received_bytes;

  RunNextModule(&batch);

  return ret;
}

pb_error_t PortInc::Init(const bess::pb::PortIncArg &arg){
  return pb_errno(0);
}

// Module* PortInc::create(const string& name, sn_port* port, int prefetch, int burst){
//   const ModuleBuilder& builder =
//        ModuleBuilder::all_module_builders().find("PortInc")->second;
//
//   string final_name = name;
//   if (ModuleBuilder::all_modules().count(name)) {
//     final_name = ModuleBuilder::GenerateDefaultName(builder.class_name(),
//                                                     builder.name_template());
//   }
//
//   Module* m = builder.CreateModule(final_name, &bess::metadata::default_pipeline);
//
//   static_cast<PortInc*>(m)->customized_init(port, prefetch, burst);
//
//   return m;
// }

void PortInc::customized_init(sn_port* port, int prefetch, int burst){
  RegisterTask(nullptr);
  port_ = port;
  prefetch_ = prefetch;
  burst_ = burst;
}

ADD_MODULE(PortInc, "port_inc", "receives packets from a port")
