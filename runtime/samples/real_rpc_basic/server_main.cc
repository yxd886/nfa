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
#include "../../actor/flow_actor.h"
#include "../../actor/flow_actor_allocator.h"
#include "../../actor/coordinator.h"
#include "../../rpc/runtime_config_allocator.h"

// #include "../../nf/base/network_function_register.h"
// #include "../../nf/pktcounter/pkt_counter.h"

using namespace bess;
using namespace std;

static constexpr int num_flow_actors = 1024*512;
static constexpr int num_rt_configs = 512;

int main(int argc, char* argv[]){

  // parse command line options
  google::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  // we do some mandatory checking to ensure that the user has given us
  // valid input
  if( (FLAGS_runtime_id == -1) ||
      (FLAGS_input_port_mac == "nil") ||
      (FLAGS_output_port_mac == "nil") ||
      (FLAGS_control_port_mac == "nil") ||
      (FLAGS_rpc_ip == "nil") ||
      (FLAGS_rpc_port == -1) ||
      (FLAGS_input_port == "") ||
      (FLAGS_output_port == "") ||
      (FLAGS_control_port == "") ||
      (FLAGS_worker_core == -1)){
    LOG(ERROR)<<"Invalid command line arguments";
    exit(-1);
  }

  // initialize dpdk environment
  nfa_init_dpdk(argv[0]);

  // create the ZeroCopyVPort in the runtime program
  sn_port input_port;
  if(input_port.init_port(FLAGS_input_port.c_str())==false){
    LOG(ERROR)<<"Fails to open input port "<<FLAGS_input_port;
    exit(EXIT_FAILURE);
  }
  else{
    LOG(INFO)<<"Successfully open input port "<<FLAGS_input_port;
  }

  sn_port output_port;
  if(output_port.init_port(FLAGS_output_port.c_str())==false){
    LOG(ERROR)<<"Fails to open output port "<<FLAGS_output_port;
    exit(EXIT_FAILURE);
  }
  else{
    LOG(INFO)<<"Successfully open output port "<<FLAGS_output_port;
  }

  sn_port control_port;
  if(control_port.init_port(FLAGS_control_port.c_str())==false){
    LOG(ERROR)<<"Fails to open control port "<<FLAGS_control_port;
    exit(EXIT_FAILURE);
  }
  else{
    LOG(INFO)<<"Successfully open control port "<<FLAGS_control_port;
  }

  // create a worker thread
  int wid = 1; // the worker id inside the program is always 1
  launch_worker(wid, FLAGS_worker_core);

  // create flow_actor_allocator, coordinator_actor and runtime_config_allocator
  flow_actor_allocator::create(num_flow_actors);
  LOG(INFO)<<"creating "<<num_flow_actors<<" flow actors";

  runtime_config_allocator::create(num_rt_configs);
  LOG(INFO)<<"creating "<<num_rt_configs<<" runtime_config";

  flow_actor_allocator* allocator = flow_actor_allocator::get();
  runtime_config_allocator* rt_allocator = runtime_config_allocator::get();
  coordinator coordinator_actor(allocator, rt_allocator);

  // create module and attach modules to the default traffic class of worker 1.
  // TODO:
}


