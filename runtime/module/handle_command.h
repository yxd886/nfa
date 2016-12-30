// The handle_command module polls the shared ring with the RPC worker thread
// and call message handler of the coordinator.
#ifndef HANDLE_COMMAND_H
#define HANDLE_COMMAND_H

#include "../bessport/module.h"
#include "../actor/coordinator.h"
#include "../rpc/ring_msg.h"

class handle_command final : public Module{

public:
  handle_command() : Module(), coordinator_actor_(0){}

  virtual struct task_result RunTask(void *arg);

  void customized_init(coordinator* coordinator_actor);

private:

  coordinator* coordinator_actor_;

  void add_input_output_runtime(llring_item* item);

  void delete_input_output_runtime(llring_item* item);
};

#endif
