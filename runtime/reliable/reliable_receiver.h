// Port the BESS port_out module here.
#ifndef RELIABLE_RECEIVER_H
#define RELIABLE_RECEIVER_H

#include "../bessport/module.h"

class reliable_receiver final : public Module {
public:
  static const gate_idx_t kNumOGates = 1;
  static const gate_idx_t kNumIGates = 1;

  reliable_receiver() {}

  virtual void ProcessBatch(bess::PacketBatch *batch);

  void customized_init();
};

#endif
