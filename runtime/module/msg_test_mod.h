// The handle_command module polls the shared ring with the RPC worker thread
// and call message handler of the coordinator.
#ifndef MSG_TEST_H
#define MSG_TEST_H

#include "../bessport/module.h"
#include "../actor/coordinator.h"
#include "../rpc/ring_msg.h"


struct test_msg{
	char msg[20];


};

class msg_test final : public Module{

public:
	msg_test() : Module(), coordinator_actor_(0){}

  virtual struct task_result RunTask(void *arg);

  void customized_init(coordinator* coordinator_actor);

private:

  coordinator* coordinator_actor_;
};

#endif
