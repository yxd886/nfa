
#include "../rpc/livesness_check_client.h"
#include <cstdlib>
#include <map>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <cassert>



using namespace std;

static constexpr long min_throughput = 2000;



class rtm_allocator{
public:
	rtm_allocator(){

		mac_prefixs.emplace("202.45.128.154",1);
		mac_prefixs.emplace("202.45.128.155",2);
		mac_prefixs.emplace("202.45.128.156",3);

	}


	int32_t get_rtm_id(string ip, int32_t local_runtime_id){

		return mac_prefixs.find(ip)->second*10+local_runtime_id;

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
			port_counter.emplace(ip,10241);
			return 10241;
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

	std::string s=rtm_name.substr(2,rtm_name.size());
	int32_t core_id=atoi(s.c_str());
	int32_t rtm_id=runtime_state.local_runtime.runtime_id;
	std::string ip=convert_uint32t_ip(runtime_state.local_runtime.rpc_ip);
	int32_t port=runtime_state.local_runtime.rpc_port;
	t= "ssh net@"+ip+" sudo nohup /home/net/nfa/runtime/samples/real_rpc_basic/server_main --runtime_id="+std::to_string(rtm_id)+" --input_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.input_port_mac)+"\" --output_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.output_port_mac)+"\" --control_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.control_port_mac)+"\" --rpc_ip=\""+convert_uint32t_ip(runtime_state.local_runtime.rpc_ip)+"\" --rpc_port="+std::to_string(port)+" --input_port=\""+rtm_name+"_iport\" --output_port=\""+rtm_name+"_oport\" --control_port=\""+rtm_name+"_cport\" --worker_core="+std::to_string(core_id)+" --service_chain=\""+service_chain+"\" > /home/net/nfa/eval/dynamic_scale_test/r"+std::to_string(rtm_id/10)+"_rt"+std::to_string(rtm_id%10)+"_log 2>&1 &";
	LOG(INFO)<<"remote command: "<<t;
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

	sleep(3);
  LivenessCheckClient checker_new(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(port)), grpc::InsecureChannelCredentials()));
  for(auto it=runtime_state.input_runtimes.begin();it!=runtime_state.input_runtimes.end();it++){
  		LivenessCheckClient checker_input(grpc::CreateChannel(
  				concat_with_colon(convert_uint32t_ip(it->second.rpc_ip),std::to_string(it->second.rpc_port)), grpc::InsecureChannelCredentials()));
  		LOG(INFO)<<"ip:"<<convert_uint32t_ip(it->second.rpc_ip);
  		LOG(INFO)<<"port:"<<it->second.rpc_port;
  		LOG(INFO)<<checker_input.SingleAddOutputRt(ip,port);

  }
  for(auto it=runtime_state.output_runtimes.begin();it!=runtime_state.output_runtimes.end();it++){


  		LOG(INFO)<<checker_new.SingleAddOutputRt(convert_uint32t_ip(it->second.rpc_ip),it->second.rpc_port);

  }
  sleep(2);
  return success;


}


bool local_open(std::string rtm_name, runtime_state runtime_state, std::string service_chain){
	std::string t;
	pid_t status;
	bool success=false;

	std::string s=rtm_name.substr(2,rtm_name.size());
	int32_t core_id=atoi(s.c_str());
	int32_t rtm_id=runtime_state.local_runtime.runtime_id;
	std::string ip=convert_uint32t_ip(runtime_state.local_runtime.rpc_ip);
	int32_t port=runtime_state.local_runtime.rpc_port;
	t="sudo nohup /home/net/nfa/runtime/samples/real_rpc_basic/server_main --runtime_id="+std::to_string(rtm_id)+" --input_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.input_port_mac)+"\" --output_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.output_port_mac)+"\" --control_port_mac=\""+convert_uint64t_mac(runtime_state.local_runtime.control_port_mac)+"\" --rpc_ip=\""+convert_uint32t_ip(runtime_state.local_runtime.rpc_ip)+"\" --rpc_port="+std::to_string(port)+" --input_port=\""+rtm_name+"_iport\" --output_port=\""+rtm_name+"_oport\" --control_port=\""+rtm_name+"_cport\" --worker_core="+std::to_string(core_id)+" --service_chain=\""+service_chain+"\" > /home/net/nfa/eval/dynamic_scale_test/rt"+std::to_string(rtm_id%10)+"_log 2>&1 &";
	LOG(INFO)<<"local command: "<<t;
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


  		LOG(INFO)<<checker_new.SingleAddOutputRt(convert_uint32t_ip(it->second.rpc_ip),it->second.rpc_port);

  }

  return success;


}





bool init(std::vector<runtime_state>& active_runtimes){

	bool success;
	runtime_state r11;
	int32_t local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.154");
	std::string rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r11.local_runtime.rpc_ip=convert_string_ip("202.45.128.154");
	r11.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.154");
	r11.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id("202.45.128.154",local_rtm_id);
	r11.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.154",local_rtm_id);
	r11.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.154",local_rtm_id);
	r11.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.154",local_rtm_id);

	success=local_open(rtm_name,r11,"null");
	if(success){
		 LOG(INFO)<<"init r11 success";
	}else{
		LOG(INFO)<<"init r11 fail";
	}

	runtime_state r12;
	local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.154");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r12.local_runtime.rpc_ip=convert_string_ip("202.45.128.154");
	r12.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.154");
	r12.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id("202.45.128.154",local_rtm_id);
	r12.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.154",local_rtm_id);
	r12.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.154",local_rtm_id);
	r12.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.154",local_rtm_id);

	success=local_open(rtm_name,r12,"null");
	if(success){
		 LOG(INFO)<<"init r12 success";
	}else{
		LOG(INFO)<<"init r12 fail";
	}

	runtime_state r13;
	local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.154");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r13.local_runtime.rpc_ip=convert_string_ip("202.45.128.154");
	r13.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.154");
	r13.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id("202.45.128.154",local_rtm_id);
	r13.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.154",local_rtm_id);
	r13.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.154",local_rtm_id);
	r13.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.154",local_rtm_id);

	success=local_open(rtm_name,r13,"null");
	if(success){
		 LOG(INFO)<<"init r13 success";
	}else{
		LOG(INFO)<<"init r13 fail";
	}

	runtime_state r14;
	local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.154");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r14.local_runtime.rpc_ip=convert_string_ip("202.45.128.154");
	r14.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.154");
	r14.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id("202.45.128.154",local_rtm_id);
	r14.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.154",local_rtm_id);
	r14.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.154",local_rtm_id);
	r14.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.154",local_rtm_id);

	success=local_open(rtm_name,r14,"null");
	if(success){
		 LOG(INFO)<<"init r14 success";
	}else{
		LOG(INFO)<<"init r14 fail";
	}

	runtime_state r15;
	local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.154");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r15.local_runtime.rpc_ip=convert_string_ip("202.45.128.154");
	r15.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.154");
	r15.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id("202.45.128.154",local_rtm_id);
	r15.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.154",local_rtm_id);
	r15.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.154",local_rtm_id);
	r15.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.154",local_rtm_id);

	success=local_open(rtm_name,r15,"null");
	if(success){
		 LOG(INFO)<<"init r15 success";
	}else{
		LOG(INFO)<<"init r15 fail";
	}

	runtime_state r16;
	local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.154");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r16.local_runtime.rpc_ip=convert_string_ip("202.45.128.154");
	r16.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.154");
	r16.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id("202.45.128.154",local_rtm_id);
	r16.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.154",local_rtm_id);
	r16.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.154",local_rtm_id);
	r16.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.154",local_rtm_id);

	success=local_open(rtm_name,r16,"null");
	if(success){
		 LOG(INFO)<<"init r16 success";
	}else{
		LOG(INFO)<<"init r16 fail";
	}

	runtime_state r21;
  local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.155");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r21.local_runtime.rpc_ip=convert_string_ip("202.45.128.155");
	r21.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.155");
	r21.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id("202.45.128.155",local_rtm_id);
	r21.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.155",local_rtm_id);
	r21.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.155",local_rtm_id);
	r21.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.155",local_rtm_id);
	success=success&&remote_open(rtm_name,r21,"pkt_counter,firewall");

	if(success){
		 LOG(INFO)<<"init r21 success";
	}else{
		LOG(INFO)<<"init r21 fail";
	}
	runtime_state r31;
  local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id("202.45.128.156");
	rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	r31.local_runtime.rpc_ip=convert_string_ip("202.45.128.156");
	r31.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id("202.45.128.156");
	r31.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id("202.45.128.156",local_rtm_id);
	r31.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr("202.45.128.156",local_rtm_id);
	r31.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr("202.45.128.156",local_rtm_id);
	r31.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr("202.45.128.156",local_rtm_id);
	success=success&&remote_open(rtm_name,r31,"pkt_counter,firewall");
	if(success){
		 LOG(INFO)<<"init r31 success";
	}else{
		LOG(INFO)<<"init r31 fail";
	}

  sleep(3);


  LivenessCheckClient checker_r11(grpc::CreateChannel(
  		concat_with_colon("202.45.128.154",std::to_string(r11.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r12(grpc::CreateChannel(
  		concat_with_colon("202.45.128.154",std::to_string(r12.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r13(grpc::CreateChannel(
  		concat_with_colon("202.45.128.154",std::to_string(r13.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r14(grpc::CreateChannel(
  		concat_with_colon("202.45.128.154",std::to_string(r14.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r15(grpc::CreateChannel(
  		concat_with_colon("202.45.128.154",std::to_string(r15.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r16(grpc::CreateChannel(
  		concat_with_colon("202.45.128.154",std::to_string(r16.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r21(grpc::CreateChannel(
  		concat_with_colon("202.45.128.155",std::to_string(r21.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  LivenessCheckClient checker_r31(grpc::CreateChannel(
  		concat_with_colon("202.45.128.156",std::to_string(r31.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));


  LOG(INFO)<<checker_r11.SingleAddOutputRt("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r11.SingleAddOutputRt("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r12.SingleAddOutputRt("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r12.SingleAddOutputRt("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r13.SingleAddOutputRt("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r13.SingleAddOutputRt("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r14.SingleAddOutputRt("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r14.SingleAddOutputRt("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r15.SingleAddOutputRt("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r15.SingleAddOutputRt("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r16.SingleAddOutputRt("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r16.SingleAddOutputRt("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r11.AddOutputMac("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r11.AddOutputMac("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r12.AddOutputMac("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r12.AddOutputMac("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r13.AddOutputMac("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r13.AddOutputMac("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r14.AddOutputMac("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r14.AddOutputMac("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r15.AddOutputMac("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r15.AddOutputMac("202.45.128.156",r31.local_runtime.rpc_port);
  LOG(INFO)<<checker_r16.AddOutputMac("202.45.128.155",r21.local_runtime.rpc_port);
  LOG(INFO)<<checker_r16.AddOutputMac("202.45.128.156",r31.local_runtime.rpc_port);



  //LOG(INFO)<<checker_r2.SetMigrationTarget("202.45.128.156",r3.local_runtime.rpc_port,10000);
  //LOG(INFO)<<checker_r3.SetMigrationTarget("202.45.128.155",r2.local_runtime.rpc_port,10000);



  runtime_state active_runtime;

  LOG(INFO)<<checker_r21.GetRuntimeState(active_runtime);
  active_runtimes.push_back(active_runtime);
  LOG(INFO)<<checker_r31.GetRuntimeState(active_runtime);
  active_runtimes.push_back(active_runtime);

	return success;

}


bool need_scale_in(const runtime_state runtime){

	string ip=convert_uint32t_ip(runtime.local_runtime.rpc_ip);
	std::string t;
	pid_t status;
	bool success=false;

	int32_t id=runtime.local_runtime.runtime_id;
	t= "python /home/net/nfa/eval/dynamic_scale_test/read_throughput_and_drop.py --ip=\""+ip+"\" --local_id="+to_string(id%10);
	//const char*a = t.c_str();
	FILE *fp;
	if((fp=popen(t.c_str(),"r"))==NULL){
		LOG(ERROR)<<"POPEN Failure";
		exit(-1);

	}

	/*status=std::system(a);
	if (-1 != status&&WIFEXITED(status)&&WEXITSTATUS(status)==0){
		success=true;
	}
	else{
    LOG(ERROR)<<"SSH Failure";
    return false;
	}*/

	uint32_t throughput;
	uint32_t drop;
	char buffer[256];
	char buffer2[256];
	/*ifstream myfile ("/home/net/nfa/eval/dynamic_scale_test/state.log");
	if(!myfile){
		LOG(ERROR)<< "Unable to open myfile";
	  exit(1); // terminate with error
	}*/
	fscanf(fp,"%s",buffer);
	fscanf(fp,"%s",buffer2);
  LOG(INFO)<<"throughput: "<<std::string(buffer);
  //LOG(INFO)<<"dropped packet: "<<std::string(buffer2);
  //myfile.close();
  //getchar();
	return atoi(buffer)<min_throughput?true:false;

}

bool need_scale_out(const runtime_state runtime){

	string ip=convert_uint32t_ip(runtime.local_runtime.rpc_ip);
	std::string t;
	pid_t status;
	bool success=false;

	int32_t id=runtime.local_runtime.runtime_id;
	LOG(INFO)<<"rtm_id: "<<id;
	//t= "cd /home/net/nfa/eval/dynamic_scale_test/ && nohup python read_throughput_and_drop.py --ip=\""+ip+"\" --local_id="+to_string(id%10)+" > state.log 2>&1 &";
	t= "python /home/net/nfa/eval/dynamic_scale_test/read_throughput_and_drop.py --ip=\""+ip+"\" --local_id="+to_string(id%10);
	FILE *fp;
	if((fp=popen(t.c_str(),"r"))==NULL){
		LOG(ERROR)<<"POPEN Failure";
		exit(-1);

	}

	/*
	const char*a = t.c_str();
	status=std::system(a);
	if (-1 != status&&WIFEXITED(status)&&WEXITSTATUS(status)==0){
		success=true;
	}
	else{
    LOG(ERROR)<<"SSH Failure";
    return false;
	}*/

	uint32_t throughput;
	uint32_t drop;
	char buffer[256];
	char buffer2[256];
	/*ifstream myfile ("/home/net/nfa/eval/dynamic_scale_test/state.log");
	if(!myfile){
		LOG(ERROR)<< "Unable to open myfile";
	  exit(1); // terminate with error
	}*/
	fscanf(fp,"%s",buffer);
	fscanf(fp,"%s",buffer2);
  //LOG(INFO)<<"throughput: "<<std::string(buffer);
  LOG(INFO)<<"dropped packet: "<<std::string(buffer2);
  //myfile.close();
  //getchar();

	return atoi(buffer2)==0?false:true;

}

bool only_one_rtm_in_server(runtime_state runtime,std::vector<runtime_state>* active_runtimes){

	int rtm_id=runtime.local_runtime.runtime_id;
	int server_id=rtm_id/10;
	int counter=0;
	for(auto it=active_runtimes->begin();it!=active_runtimes->end();it++){
		if(it->local_runtime.runtime_id/10==server_id){
			counter++;
		}
	}
	return counter==1?true:false;



}

void scale_in(runtime_state runtime,std::vector<runtime_state>* active_runtimes){


	if(runtime.local_runtime.rpc_port==10241){
		LOG(INFO)<<"this is the pirme rtm";
		return;
	}
	LOG(INFO)<<"scale in";
	std::string ip=convert_uint32t_ip(runtime.local_runtime.rpc_ip);

  LivenessCheckClient checker_source(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(runtime.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));

  LOG(INFO)<<checker_source.GetRuntimeState(runtime);
  checker_source.MigrateTo(convert_uint32t_ip(runtime.migration_target.rpc_ip),10241,runtime.flow_state.active_flows);

  sleep(2);
  checker_source.ShutdownRuntime();

  for(auto it=active_runtimes->begin();it!=active_runtimes->end();it++){
  	if(it->local_runtime==runtime.local_runtime){

  		it=active_runtimes->erase(it);
  		break;
  	}
  }

  //getchar();

}


void scale_out(runtime_state runtime,std::vector<runtime_state>* active_runtimes){

	if(active_runtimes->size()>=6)
		return;
	LOG(INFO)<<"scale out";
	std::string ip=convert_uint32t_ip(runtime.local_runtime.rpc_ip);


	LivenessCheckClient checker_source(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(runtime.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
	LOG(INFO)<<checker_source.GetRuntimeState(runtime);
	int32_t local_rtm_id=static_allocator::get_allocator().next_availiable_local_rtm_id(ip);
	std::string rtm_name=static_allocator::get_allocator().get_rtm_name(local_rtm_id);
	runtime.local_runtime.rpc_ip=convert_string_ip(ip);
	runtime.local_runtime.rpc_port=static_allocator::get_allocator().next_availiable_port_id(ip);
	runtime.local_runtime.runtime_id=static_allocator::get_allocator().get_rtm_id(ip,local_rtm_id);
	runtime.local_runtime.input_port_mac=static_allocator::get_allocator().next_availiable_input_mac_addr(ip,local_rtm_id);
	runtime.local_runtime.output_port_mac=static_allocator::get_allocator().next_availiable_output_mac_addr(ip,local_rtm_id);
	runtime.local_runtime.control_port_mac=static_allocator::get_allocator().next_availiable_control_mac_addr(ip,local_rtm_id);

	remote_open(rtm_name,runtime,"pkt_counter,firewall");

	LOG(INFO)<<checker_source.SetMigrationTarget(ip,runtime.local_runtime.rpc_port,runtime.flow_state.active_flows/2);
	LOG(INFO)<<"IP:"<<ip;
	LOG(INFO)<<"port:"<<runtime.local_runtime.rpc_port;
	LOG(INFO)<<"runtime.flow_state.active_flows/2: "<<runtime.flow_state.active_flows/2;

	LOG(INFO)<<checker_source.MigrateTo(ip,runtime.local_runtime.rpc_port,runtime.flow_state.active_flows/2);

  LivenessCheckClient checker_dest(grpc::CreateChannel(
  		concat_with_colon(ip,std::to_string(runtime.local_runtime.rpc_port)), grpc::InsecureChannelCredentials()));
  runtime_state tmp;
  checker_dest.GetRuntimeState(tmp);
  checker_dest.SetMigrationTarget(ip,10241,10000);
  active_runtimes->push_back(tmp);
  sleep(2);

}


int main(int argc, char** argv) {

	std::vector<runtime_state>active_runtimes;
	active_runtimes.reserve(100);
	assert(active_runtimes.size()==0);

	init(active_runtimes);

  while(1){


  	sleep(1);
  	for(auto it=active_runtimes.begin();it!=active_runtimes.end();it++){

  		if(need_scale_out(*it)){

			scale_out(*it,&active_runtimes);
  			continue;
  		}


  		if(need_scale_in(*it)){
  			LOG(INFO)<<"need scale in";



			scale_in(*it,&active_runtimes);


  			continue;
  		}


  	}



  }

  return 0;
}













