#include "random_dropper.h"
#include <glog/logging.h>

void random_dropper::ProcessBatch(bess::PacketBatch *batch) {
  LOG(INFO)<<"random_dropper receives "<<batch->cnt()<<" packets.";
  RunNextModule(batch);
}

void random_dropper::customized_init(){

}

ADD_MODULE(random_dropper, "random_dropper", "randomly drop packets")
