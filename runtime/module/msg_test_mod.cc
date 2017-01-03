//
#include "msg_test_mod.h"
#include "../bessport/kmod/llring.h"
#include <string.h>

void msg_test::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
}

struct task_result msg_test::RunTask(void *arg){
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };
  test_msg msg;
  memset(msg.msg,0,sizeof(msg.msg));
  strcpy(msg.msg,"hello world!");
  coordinator_actor_->reliables_.find(1)->second.reliable_send<test_msg,coordinator_messages::es_scheduler_pkt_batch>(0,1,1,es_scheduler_pkt_batch_t::value,&msg);
  return ret;
}

ADD_MODULE(msg_test, "msgtest", "test msg")
