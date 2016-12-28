// Port the BESS port_out module here.
#ifndef RANDOM_DROPPER_H
#define RANDOM_DROPPER_H

#include "../bessport/module.h"

class random_dropper final : public Module {
public:
  static const gate_idx_t kNumOGates = 1;
  static const gate_idx_t kNumIGates = 1;

  random_dropper() {}

  virtual void ProcessBatch(bess::PacketBatch *batch);

  void customized_init();
};

#endif
