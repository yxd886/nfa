#include "port_out.h"
#include "../bessport/message.h"

void PortOut::ProcessBatch(bess::PacketBatch *batch) {
  /* TODO: choose appropriate out queue */
  const uint8_t qid = 0;

  uint64_t sent_bytes = 0;
  int sent_pkts;

  sent_pkts = port_->SendPackets(qid, batch->pkts(), batch->cnt());

  const packet_dir_t dir = PACKET_DIR_OUT;

  for (int i = 0; i < sent_pkts; i++)
    sent_bytes += batch->pkts()[i]->total_len();

  port_->queue_stats[dir][qid].packets += sent_pkts;
  port_->queue_stats[dir][qid].dropped += (batch->cnt() - sent_pkts);
  port_->queue_stats[dir][qid].bytes += sent_bytes;


  if (sent_pkts < batch->cnt()) {
    bess::Packet::Free(batch->pkts() + sent_pkts, batch->cnt() - sent_pkts);
  }
}

pb_error_t PortOut::Init(const bess::pb::PortIncArg &arg){
  return pb_errno(0);
}

// Module* PortOut::create(const string& name, sn_port* port){
//   const ModuleBuilder& builder =
//           ModuleBuilder::all_module_builders().find("PortInc")->second;
//
//   string final_name = name;
//   if (ModuleBuilder::all_modules().count(name)) {
//     final_name = ModuleBuilder::GenerateDefaultName(builder.class_name(),
//                                                     builder.name_template());
//   }
//
//   Module* m = builder.CreateModule(final_name, &bess::metadata::default_pipeline);
//
//   static_cast<PortOut*>(m)->customized_init(port);
//
//   return m;
// }

void PortOut::customized_init(sn_port* port){
  port_ = port;
}

ADD_MODULE(PortOut, "port_out", "sends pakets to a port")
