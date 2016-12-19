// This module is where we poll input/output port and schedule
// execution_context actors.

#ifndef EC_SCHEDULER_H
#define EC_SCHEDULER_H

#include <string>

#include "../port/sn_port.h"
#include "../bessport/module.h"
#include "../bessport/module_msg.pb.h"
#include "../bessport/message.h"

using std::string;

class coordinator;

class ec_scheduler final : public Module {
public:

  static const gate_idx_t kNumOGates = 2;
  static const gate_idx_t kNumIGates = 1;

  ec_scheduler() : Module(), coordinator_actor_(0){}

  // Fake init function.
  pb_error_t Init(const bess::pb::PortIncArg &arg);

  virtual void ProcessBatch(bess::PacketBatch *batch);

  void customized_init(coordinator* coordinator_actor);

private:
  coordinator* coordinator_actor_;
};

#endif
