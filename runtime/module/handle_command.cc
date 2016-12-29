//
#include "handle_command.h"
#include "../bessport/kmod/llring.h"
#include "../rpc/ring_msg.h"

void handle_command::customized_init(coordinator* coordinator_actor){
  RegisterTask(nullptr);
  coordinator_actor_ = coordinator_actor;
}

struct task_result handle_command::RunTask(void *arg){
  struct task_result ret;
  ret = (struct task_result){
      .packets = 0, .bits = 0,
  };

  void* dequeue_output[1];

  int flag = llring_sc_dequeue(coordinator_actor_->peek_rpc2worker_ring(), dequeue_output);

  if(unlikely(flag == 0)){
    // do the processing

    llring_item* item = static_cast<llring_item*>(dequeue_output[0]);

    LOG(INFO) << "Receive "<<opcode2string(item->op_code)<<" ring message.";
    print_config(item->rt_config);
    LOG(INFO) << "migration_qouta-> "<<item->migration_qouta;
    print_stat(item->op_code, item->stat);

    llring_sp_enqueue(coordinator_actor_->peek_worker2rpc_ring(), static_cast<void*>(item));
  }

  return ret;
}

ADD_MODULE(handle_command, "handle_command", "handle rpc command received from the rpc thread")
