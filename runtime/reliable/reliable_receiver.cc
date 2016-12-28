#include "reliable_receiver.h"

void reliable_receiver::ProcessBatch(bess::PacketBatch *batch) {
  LOG(INFO)<<"reliable_receiver receives "<<batch->cnt()<<" packets.";
  RunNextModule(batch);
}

void reliable_receiver::customized_init(){

}

ADD_MODULE(reliable_receiver, "reliable_receiver", "reliably receive packets")
