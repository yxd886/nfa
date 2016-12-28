#include <thread>
#include <chrono>
#include <cassert>

#include <glog/logging.h>

#include "../../nfaflags.h"
#include "../../nfadpdk.h"
#include "../../bessport/worker.h"
#include "../../bessport/traffic_class.h"
#include "../../bessport/task.h"
#include "../../bessport/scheduler.h"
#include "../../port/sn_port.h"
#include "../../module/port_inc.h"
#include "../../module/port_out.h"
#include "../../module/ec_scheduler.h"
#include "../../module/sink.h"
#include "../../module/timers.h"
#include "../../module/create.h"
#include "../../reliable/random_dropper.h"
#include "../../reliable/reliable_receiver.h"
#include "../../reliable/reliable_sender.h"
#include "../../actor/flow_actor.h"
#include "../../actor/flow_actor_allocator.h"
#include "../../actor/coordinator.h"

using namespace bess;
using namespace std;

static constexpr int num_flow_actors = 1024*512;

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  nfa_init_dpdk(argv[0]);

  flow_actor_allocator::create(num_flow_actors);
  LOG(INFO)<<"creating "<<num_flow_actors<<" flow actors";

  flow_actor_allocator* allocator = flow_actor_allocator::get();
  coordinator coordinator_actor(allocator);
  LOG(INFO)<<"The size of flow_actor is "<<sizeof(flow_actor);

  if((string(FLAGS_input_port)=="")||(string(FLAGS_output_port)=="")){
    LOG(ERROR)<<"The name of intput/output port must be specified";
    exit(EXIT_FAILURE);
  }

  sn_port input_port;
  sn_port output_port;

  if(input_port.init_port(FLAGS_input_port.c_str())==false){
    LOG(ERROR)<<"Fails to open input port "<<FLAGS_input_port;
    exit(EXIT_FAILURE);
  }
  else{
    LOG(INFO)<<"Successfully open input port "<<FLAGS_input_port;
  }

  if(output_port.init_port(FLAGS_output_port.c_str())==false){
    LOG(ERROR)<<"Fails to open output port "<<FLAGS_output_port;
    exit(EXIT_FAILURE);
  }
  else{
    LOG(INFO)<<"Successfully open output port "<<FLAGS_output_port;
  }

  launch_worker(3,3);
  int wid = 3;

  Module* mod_reliable_sender = create_module<reliable_sender>("reliable_sender", "mod_reliable_sender", &input_port);

  Module* mod_reliable_receiver = create_module<reliable_receiver>("reliable_receiver", "mod_reliable_receiver");

  Module* mod_random_dropper = create_module<random_dropper>("random_dropper", "mod_random_dropper");

  Module* mod_timers = create_module<timers>("timers", "mod_timer", &coordinator_actor);

  bool flag = mod_reliable_sender->ConnectModules(0, mod_random_dropper, 0);
  if(flag!=0){
    LOG(ERROR)<<"Error connecting mod_reliable_sender with mod_random_dropper";
    exit(-1);
  }
  LOG(INFO)<<"mod_reliable_sender is connected to mod_random_dropper";

  flag = mod_random_dropper->ConnectModules(0, mod_reliable_receiver, 0);
  if(flag!=0){
    LOG(ERROR)<<"Error connecting mod_random_dropper with mod_reliable_receiver";
    exit(-1);
  }
  LOG(INFO)<<"mod_random_dropper is connected to mod_reliable_receiver";

  flag = mod_reliable_receiver->ConnectModules(0, mod_reliable_sender, 0);
  if(flag!=0){
    LOG(ERROR)<<"Error connecting mod_reliable_receiver with mod_reliable_sender";
    exit(-1);
  }
  LOG(INFO)<<"mod_reliable_receiver is connected to mod_reliable_sender";

  Task* t = mod_reliable_sender->tasks()[0];
  if(t==nullptr){
    LOG(ERROR)<<"mod_reliable_sender has no task";
    exit(-1);
  }

  Task* t_timers = mod_timers->tasks()[0];
  if(t_timers == nullptr){
    LOG(ERROR)<<"mod_timers has no task";
    exit(-1);
  }

  bess::LeafTrafficClass* tc =
            workers[wid]->scheduler()->default_leaf_class();
  if (!tc) {
    LOG(ERROR)<<"worker "<<wid<<" has no leaf traffic class";
    exit(-1);
  }

  tc->AddTask(t);
  tc->AddTask(t_timers);

  resume_all_workers();

  while(true){
    std::this_thread::sleep_for(std::chrono::seconds(50));
  }
}
