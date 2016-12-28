// Port the BESS port_inc module here.
#ifndef RELIABLE_SENDER_H
#define RELIABLE_SENDER_H

#include "../port/sn_port.h"
#include "../bessport/module.h"

class reliable_sender final : public Module {
public:
  static const gate_idx_t kNumIGates = 1;

  static const gate_idx_t kNumOGates = 1;

  reliable_sender() : port_(0){}

  virtual struct task_result RunTask(void *arg);

  virtual void ProcessBatch(bess::PacketBatch *batch);

  void customized_init(sn_port* port);

private:
  sn_port* port_;
};

#endif
