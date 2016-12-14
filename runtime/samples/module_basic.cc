#include <thread>
#include <chrono>

#include <glog/logging.h>

#include "../nfaflags.h"
#include "../nfadpdk.h"
#include "../bessport/worker.h"
#include "../bessport/traffic_class.h"
#include "../module/port_inc.h"
#include "../module/port_out.h"
#include "../module/create.h"
#include "../port/sn_port.h"
#include "../bessport/task.h"
#include "../bessport/scheduler.h"

using namespace bess;
using namespace std;

int main(int argc, char* argv[]){
  google::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  nfa_init_dpdk(argv[0]);

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

  bool flag = mod_port_inc->ConnectModules(0, mod_port_out, 0);
  if(flag!=0){
    LOG(ERROR)<<"Error connecting mod_port_inc with mod_port_out";
    exit(-1);
  }
  LOG(INFO)<<"mod_port_inc is connected to mod_port_out";

  Task* t = mod_port_inc->tasks()[0];
  if(t==nullptr){
    LOG(ERROR)<<"mod_port_inc has no task";
    exit(-1);
  }

  bess::LeafTrafficClass* tc =
            workers[wid]->scheduler()->default_leaf_class();
  if (!tc) {
    LOG(ERROR)<<"worker "<<wid<<" has no leaf traffic class";
    exit(-1);
  }

  tc->AddTask(t);

  resume_all_workers();

  while(true){
    std::this_thread::sleep_for(std::chrono::seconds(50));
  }
}
