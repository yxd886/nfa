#include <thread>
#include <chrono>
#include <cassert>
#include <vector>

#include <glog/logging.h>

#include "../nfaflags.h"
#include "../nfadpdk.h"
#include "../bessport/worker.h"
#include "../bessport/traffic_class.h"
#include "../bessport/task.h"
#include "../bessport/scheduler.h"
#include "../port/sn_port.h"
#include "../module/port_inc.h"
#include "../module/port_out.h"
#include "../module/ec_scheduler.h"
#include "../module/sink.h"
#include "../module/timers.h"
#include "../module/create.h"
#include "../actor/flow_actor.h"
#include "../actor/flow_actor_allocator.h"
#include "../actor/coordinator.h"

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

  std::vector<network_function_base*> service_chain_;
  service_chain_.push_back(new network_function_derived<pkt_counter, pkt_counter_fs>(allocator->get_max_actor()));
  service_chain_.push_back(new network_function_derived<firewall, firewall_fs>(allocator->get_max_actor()));
  service_chain_.push_back(new network_function_derived<flow_monitor, flow_monitor_fs>(allocator->get_max_actor()));


  coordinator coordinator_actor(allocator,&service_chain_);

  /*flow_actor* a0 = allocator->allocate();
  flow_actor* a1 = allocator->allocate();
  flow_actor* a2 = allocator->allocate();
  flow_actor* a3 = allocator->allocate();
  flow_actor* a4 = allocator->allocate();
  flow_actor* a5 = allocator->allocate();

  assert(a0->get_id() == 0);
  assert(a1->get_id() == 1);
  assert(a2->get_id() == 2);
  assert(a3->get_id() == 3);
  assert(a4->get_id() == 4);
  assert(a5 == nullptr);

  assert(allocator->deallocate(a4) == true);
  assert(allocator->deallocate(a3) == true);
  assert(allocator->deallocate(a2) == true);
  assert(allocator->deallocate(a1) == true);
  assert(allocator->deallocate(a0) == true);
  assert(allocator->deallocate(a5) == false);

  a0 = allocator->allocate();
  a1 = allocator->allocate();
  a2 = allocator->allocate();
  a3 = allocator->allocate();
  a4 = allocator->allocate();
  a5 = allocator->allocate();

  assert(a0->get_id() == 4);
  assert(a1->get_id() == 3);
  assert(a2->get_id() == 2);
  assert(a3->get_id() == 1);
  assert(a4->get_id() == 0);
  assert(a5 == nullptr);

  LOG(INFO)<<"pass flow_actor_allocator test";*/

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

  Module* mod_port_inc = create_module<PortInc>("PortInc", "mod_port_inc", &input_port, 0, 32);
  Module* mod_port_out = create_module<PortOut>("PortOut", "mod_port_out", &output_port);
  Module* mod_ec_scheduler = create_module<ec_scheduler>("ec_scheduler", "mod_ec_scheduler", &coordinator_actor);
  Module* mod_sink = create_module<Sink>("Sink", "mod_sink");

  Module* mod_timers = create_module<timers>("timers", "mod_timer", &coordinator_actor);

  bool flag = mod_port_inc->ConnectModules(0, mod_ec_scheduler, 0);
  if(flag!=0){
    LOG(ERROR)<<"Error connecting mod_port_inc with mod_ec_scheduler";
    exit(-1);
  }
  LOG(INFO)<<"mod_port_inc is connected to mod_ec_scheduler";

  flag = mod_ec_scheduler->ConnectModules(0, mod_port_out, 0);
  if(flag!=0){
    LOG(ERROR)<<"Error connecting mod_ec_scheduler with mod_port_out";
    exit(-1);
  }
  LOG(INFO)<<"mod_ec_scheduler is connected to mod_port_out";

  flag = mod_ec_scheduler->ConnectModules(1, mod_sink, 0);
  if(flag!=0){
    LOG(ERROR)<<"Error connecting ogate 1 of mod_ec_scheduler with mod_sink";
    exit(-1);
  }
  LOG(INFO)<<"ogate 1 of mod_ec_scheduler is connected with mod_sink";

  Task* t = mod_port_inc->tasks()[0];
  if(t==nullptr){
    LOG(ERROR)<<"mod_port_inc has no task";
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
