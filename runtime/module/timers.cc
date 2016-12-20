//
#include "timers.h"
#include "../actor/fixed_timer.h"

void timers::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
}

struct task_result timers::RunTask(void *arg) {
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  for(size_t i=0; i<bess::PacketBatch::kMaxBurst; i++){
    //trigger at most kMaxBurst timers on each invoke.
    if(unlikely(timeout_occur(coordinator_actor_->peek_idle_flow_check_list(), ctx.current_ns()))){
      trigger_timer(coordinator_actor_->peek_idle_flow_check_list());
    }
    else{
      continue;
    }
  }

  return ret;
}

ADD_MODULE(timers, "timers module", "check the timeout for fixed timers")
