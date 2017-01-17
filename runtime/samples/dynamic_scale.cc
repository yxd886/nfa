
#include "../rpc/livesness_check_client.h"
#include <cstdlib>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



using namespace std;

static constexpr long max_throughput = 2000000;
static constexpr long min_throughput = 20000;


class rtm_allocator{
public:
	rtm_allocator(){

		mac_prefixs.emplace("202.45.128.154",1);
		mac_prefixs.emplace("202.45.128.155",2);
		mac_prefixs.emplace("202.45.128.156",3);

	}


	int32_t next_availiable_rtm_id(){

		return rtm_counter++;

	}


	int32_t next_availiable_local_rtm_id(std::string ip){

		auto it=runtime_no.find(ip);
		if(it==runtime_no.end()){
			runtime_no.emplace(ip,1);
			return 1;
		}else{
			it->second++;
			return it->second;

		}

	}


	std::string get_rtm_name(int32_t local_rtm_id){

		std::string t;
		t="rt"+std::to_string(local_rtm_id);
		return t;

	}




	int32_t next_availiable_port_id(std::string ip){
		auto it=port_counter.find(ip);
		if(it==port_counter.end()){
			port_counter.emplace(ip,10240);
			return 10240;
		}else{
			it->second++;
			return it->second;
		}

	}

	uint64_t next_availiable_input_mac_addr(std::string ip,int32_t rtm_id){

		std::string t;
		auto it=mac_prefixs.find(ip);
		if(it==mac_prefixs.end()){
			LOG(INFO)<<"allocate mac addr error";
			return 0;
		}
		int32_t mac_prefix=it->second;
		t="52:54:"+std::to_string(it->second)+":"+std::to_string(rtm_id)+":00:01";
		return convert_string_mac(t);

	}

	uint64_t next_availiable_output_mac_addr(std::string ip,int32_t rtm_id){

		std::string t;
		auto it=mac_prefixs.find(ip);
		if(it==mac_prefixs.end()){
			LOG(INFO)<<"allocate mac addr error";
			return 0;
		}
		int32_t mac_prefix=it->second;
		t="52:54:"+std::to_string(it->second)+":"+std::to_string(rtm_id)+":00:02";
		return convert_string_mac(t);

	}

	uint64_t next_availiable_control_mac_addr(std::string ip,int32_t rtm_id){

		std::string t;
		auto it=mac_prefixs.find(ip);
		if(it==mac_prefixs.end()){
			LOG(INFO)<<"allocate mac addr error";
			return 0;
		}
		int32_t mac_prefix=it->second;
		t="52:54:"+std::to_string(it->second)+":"+std::to_string(rtm_id)+":00:03";
		return convert_string_mac(t);

	}


	int32_t rtm_counter=1; //<ip,runtime_id>
	std::map<std::string,int32_t> port_counter; //<ip,port>
	std::map<std::string,int32_t> mac_prefixs;
	std::map<std::string,int32_t> runtime_no;

};


class static_allocator{
public:
	static_allocator(){}
	static rtm_allocator& get_allocator(){
		static rtm_allocator t;
		return t;

	}
};




bool remote_open(std::string rtm_name, runtime_state runtime_state, std::string service_chain){
	std::string t;
	pid_t status;
	bool success=false;


	int32_t rtm_id=runtime_state.local_runtime.runtime_id;
	std::string ip=convert_uint32t_ip(runtime_state.local_runtime.rpc_ip);
	int32_t port=runtime_state.local_runtime.rpc_port;
	t= "ssh net@"+ip+" sudo nohup /home/net/nfa/runtime/sample/real_rpc_basic/server_main --runtime_id="+std::to_string(rtm_id)+" --input_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.input_port_mac)+"\" --output_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.output_port_mac)+"\" --control_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.control_port_mac)+"\" --rpc_ip=\""+convert_uint32t_ip(runtime_state.local_runtime.rpc_ip)+"\" --rpc_port="+std::to_string(port)+" --input_port=\""+rtm_name+"_iport\" --output_port=\""+rtm_name+"_oport\" --control_port=\""+rtm_name+"_cport\" --worker_core="+std::to_string(rtm_id)+" --service_chain=\""+service_chain+"\" &";
	const char*a = t.c_str();
	status=std::system(a);
	if (-1 != status&&WIFEXITED(status)&&WEXITSTATUS(status)==0){
		//successful
		//printf("run shell script successfully!\n");
		//return true;
		success=true;
	}
	else{
		//failure
		//aout(self)<<"open failure,try again"<<endl;
		//self->delayed_send(self,HEARTBEAT_TIME,start_atom::value,ip,number);
    LOG(ERROR)<<"SSH Failure";
    return false;
	}

  LivenessCheckClient checker_new(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(port)), grpc::InsecureChannelCredentials()));
  for(auto it=runtime_state.input_runtimes.begin();it!=runtime_state.input_runtimes.end();it++){
  	LivenessCheckClient checker_input(grpc::CreateChannel(
  	  		concat_with_colon(convert_uint32t_ip(it->second.rpc_ip),std::to_string(it->second.rpc_port)), grpc::InsecureChannelCredentials()));
  	LOG(INFO)<<checker_input.SingleAddOutputRt(ip,port);

  }
  for(auto it=runtime_state.output_runtimes.begin();it!=runtime_state.output_runtimes.end();it++){


  	LOG(INFO)<<checker_new.SingleAddOutputRt(convert_uint32t_ip(it->second.rpc_ip),std::to_string(it->second.rpc_port));

  }

  return success;


}







bool init(std::vector<runtime_state>& active_runtimes){

	bool success;
	runtime_state r1;
	int32_t local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.154");
	std::string rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r1.local_runtime.rpc_ip=convert_string_ip("202.45.128.154");
	r1.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.154");
	r1.local_runtime.runtime_id=static_allocator::get_allocator().next_availiable_rtm_id();
	r1.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.154",local_rtm_id);
	r1.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.154",local_rtm_id);
	r1.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.154",local_rtm_id);

	success=remote_open(rtm_name,r1,"null");

	runtime_state r2;
  local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.155");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r2.local_runtime.rpc_ip=convert_string_ip("202.45.128.155");
	r2.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.155");
	r2.local_runtime.runtime_id=static_allocator::get_allocator().next_availiable_rtm_id();
	r2.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.155",local_rtm_id);
	r2.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.155",local_rtm_id);
	r2.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.155",local_rtm_id);
	success=success&&remote_open(rtm_name,r2,"pkt_counter,firewall");

	runtime_state r3;
  local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.156");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r3.local_runtime.rpc_ip=convert_string_ip("202.45.128.156");
	r3.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.156");
	r3.local_runtime.runtime_id=static_allocator::get_allocator().next_availiable_rtm_id();
	r3.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.156",local_rtm_id);
	r3.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.156",local_rtm_id);
	r3.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.156",local_rtm_id);
	success=success&&remote_open(rtm_name,r3,"pkt_counter,firewall");

	return success;



  LivenessCheckClient checker_r1(grpc::CreateChannel(
  		concat_with_colon("202.45.128.154",std::to_string(r1.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r2(grpc::CreateChannel(
  		concat_with_colon("202.45.128.155",std::to_string(r2.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r3(grpc::CreateChannel(
  		concat_with_colon("202.45.128.156",std::to_string(r3.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));


  LOG(INFO)<<checker_r1.SingleAddOutputRt("202.45.128.155",r2.local_runtime.rpc_port);
  LOG(INFO)<<checker_r1.SingleAddOutputRt("202.45.128.156",r3.local_runtime.rpc_port);
  LOG(INFO)<<checker_r1.AddOutputMac("202.45.128.155",r2.local_runtime.rpc_port);
  LOG(INFO)<<checker_r1.AddOutputMac("202.45.128.156",r3.local_runtime.rpc_port);
  LOG(INFO)<<checker_r2.AddInputMac("202.45.128.154",r1.local_runtime.rpc_port);
  LOG(INFO)<<checker_r3.AddInputMac("202.45.128.154",r1.local_runtime.rpc_port);


  LOG(INFO)<<checker_r2.SetMigrationTarget("202.45.128.156",r3.local_runtime.rpc_port,10000);
  LOG(INFO)<<checker_r3.SetMigrationTarget("202.45.128.155",r2.local_runtime.rpc_port,10000);



  runtime_state active_runtime;

  LOG(INFO)<<checker_r1.GetRuntimeState(active_runtime);
  active_runtimes.push_back(active_runtime);
  LOG(INFO)<<checker_r2.GetRuntimeState(active_runtime);
  active_runtimes.push_back(active_runtime);
  LOG(INFO)<<checker_r3.GetRuntimeState(active_runtime);
  active_runtimes.push_back(active_runtime);

}


bool need_scale_in(const runtime_state runtime){
	string ip=convert_uint32t_ip(runtime.local_runtime.rpc_ip);
	long begin=0;
	long end=0;
  LivenessCheckClient checker(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(runtime.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));

  runtime_state tmp;
  checker.GetRuntimeState(tmp);
  begin=tmp.port_state.input_port_incoming_pkts;
  sleep(1);
  checker.GetRuntimeState(tmp);
  end=tmp.port_state.input_port_incoming_pkts;

	return (end-begin)<min_throughput?true:false;

}

bool need_scale_out(const runtime_state runtime){

	string ip=convert_uint32t_ip(runtime.local_runtime.rpc_ip);
	long begin=0;
	long end=0;
  LivenessCheckClient checker(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(runtime.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));

  runtime_state tmp;
  checker.GetRuntimeState(tmp);
  begin=tmp.port_state.input_port_incoming_pkts;
  sleep(1);
  checker.GetRuntimeState(tmp);
  end=tmp.port_state.input_port_incoming_pkts;

	return (end-begin)>max_throughput?true:false;

}


void scale_in(runtime_state runtime,std::vector<runtime_state>& active_runtimes){


	std::string ip=convert_uint32t_ip(runtime.local_runtime.rpc_ip);

  LivenessCheckClient checker_source(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(runtime.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));

  checker_source.MigrateTo(convert_uint32t_ip(runtime.migration_target.rpc_ip),runtime.migration_target.rpc_port,runtime.flow_state.active_flows);

  checker_source.ShutdownRuntime();

  for(auto it=active_runtimes.begin();it!=active_runtimes.end();it++){
  	if(it->local_runtime==runtime.local_runtime){

  		it=active_runtimes.erase(it);
  		break;
  	}
  }



}


void scale_out(runtime_state runtime,std::vector<runtime_state>& active_runtimes){

	std::string ip=convert_uint32t_ip(runtime.local_runtime.rpc_ip);

  LivenessCheckClient checker_source(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(runtime.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
	int32_t local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id(ip);
	std::string rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	runtime.local_runtime.rpc_ip=convert_string_ip(ip);
	runtime.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id(ip);
	runtime.local_runtime.runtime_id=static_allocator::get_allocator().next_availiable_rtm_id();
	runtime.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr(ip,local_rtm_id);
	runtime.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr(ip,local_rtm_id);
	runtime.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr(ip,local_rtm_id);

	remote_open(rtm_name,runtime,"pkt_counter,firewall");

	checker_source.SetMigrationTarget(ip,runtime.local_runtime.rpc_port,runtime.flow_state.active_flows/2);
	checker_source.MigrateTo(ip,runtime.local_runtime.rpc_port,runtime.flow_state.active_flows/2);

  LivenessCheckClient checker_dest(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(runtime.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  runtime_state tmp;
  checker_dest.GetRuntimeState(tmp);
  active_runtimes.push_back(tmp);


}


int main(int argc, char** argv) {

	std::vector<runtime_state>active_runtimes;

	init(active_runtimes);

  while(1){

  	for(auto it=active_runtimes.begin();it!=active_runtimes.end();it++){

  		if(need_scale_out(*it)){
  			scale_out(*it,active_runtimes);
  			continue;
  		}


  		if(need_scale_in(*it)){
  			scale_in(*it,active_runtimes);
  			continue;
  		}


  	}



  }

  return 0;
}













