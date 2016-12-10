#ifndef RING_MSG_H
#define RING_MSG_H

#include <string>
#include <sstream>

#include "../bessport/nfa_msg.grpc.pb.h"
#include "../bessport/mem_alloc.h"

using std::string;
using std::stringstream;
using std::istreambuf_iterator;

using nfa_msg::RuntimeConfig;

struct runtime_config{
  int32_t runtime_id;
  uint64_t input_port_mac;
  uint64_t output_port_mac;
  uint64_t control_port_mac;
  uint32_t rpc_ip;
  int32_t rpc_port;

  inline bool operator==(const runtime_config& rhs){
    if( (this->runtime_id == rhs.runtime_id) &&
        (this->input_port_mac == rhs.input_port_mac) &&
        (this->output_port_mac == rhs.output_port_mac) &&
        (this->control_port_mac == rhs.control_port_mac) &&
        (this->rpc_ip == rhs.rpc_ip) &&
        (this->rpc_port == rhs.rpc_port) ){
      return true;
    }
    else{
      return false;
    }
  }

  inline bool operator!=(const runtime_config& rhs){
    return !((*this) == rhs);
  }
};

inline uint64_t convert_string_mac(const string& mac){
  stringstream ss(mac);
  uint64_t a,b,c,d,e,f;
  char ch;
  ss >> a >> ch >> b >> ch >> c >> ch >> d >> ch >> e >>ch >> f;
  return  (
           ((a<<40)&0xFF0000000000) |
           ((b<<32)&0x00FF00000000) |
           ((c<<24)&0x0000FF000000) |
           ((d<<16)&0x000000FF0000) |
           ((e<< 8)&0x00000000FF00) |
           ( f     &0x0000000000FF));
}

inline uint32_t convert_string_ip(const string& ip){
  stringstream ss(ip);
  uint32_t a,b,c,d;
  char ch;
  ss >> a >> ch >> b >> ch >> c >> ch >> d;
  return  (((a<<24)&0xFF000000) |
           ((b<<16)&0x00FF0000) |
           ((c<< 8)&0x0000FF00) |
           ( d     &0x000000FF));
}

inline string convert_uint64t_mac(uint64_t mac){
  uint32_t a,b,c,d,e,f;
  a = (mac>>40)&0x000000FF;
  b = (mac>>32)&0x000000FF;
  c = (mac>>24)&0x000000FF;
  d = (mac>>16)&0x000000FF;
  e = (mac>>8)&0x000000FF;
  f = mac&0x000000FF;
  stringstream ss("");

  ss<<a<<":"<<b<<":"<<c<<":"<<d<<":"<<e<<":"<<f;
  string ret(istreambuf_iterator<char>(ss), {});
  return ret;
}

inline string convert_uint32t_ip(uint32_t ip){
  uint32_t a,b,c,d;
  a = (ip>>24)&0x000000FF;
  b = (ip>>16)&0x000000FF;
  c = (ip>>8)&0x000000FF;
  d = ip&0x000000FF;
  stringstream ss("");
  ss<<a<<"."<<b<<"."<<c<<"."<<d;
  string ret(istreambuf_iterator<char>(ss), {});
  return ret;
}

inline runtime_config protobuf2local(const RuntimeConfig &protobuf_msg){
  runtime_config rc;
  rc.runtime_id = protobuf_msg.runtime_id();
  rc.input_port_mac = convert_string_mac(protobuf_msg.input_port_mac());
  rc.output_port_mac = convert_string_mac(protobuf_msg.output_port_mac());
  rc.control_port_mac = convert_string_mac(protobuf_msg.control_port_mac());
  rc.rpc_ip = convert_string_ip(protobuf_msg.rpc_ip());
  rc.rpc_port = protobuf_msg.rpc_port();
  return rc;
}

inline RuntimeConfig local2protobuf(runtime_config local_msg){
  RuntimeConfig rc;
  rc.set_runtime_id(local_msg.runtime_id);
  rc.set_input_port_mac(convert_uint64t_mac(local_msg.input_port_mac));
  rc.set_output_port_mac(convert_uint64t_mac(local_msg.output_port_mac));
  rc.set_control_port_mac(convert_uint64t_mac(local_msg.control_port_mac));
  rc.set_rpc_ip(convert_uint32t_ip(local_msg.rpc_ip));
  rc.set_rpc_port(local_msg.rpc_port);
  return rc;
}

struct storage_stat{
  uint64_t replication_source_runtime_id;
  uint64_t num_of_flow_replicas;
  uint64_t total_replay_time;
};

struct runtime_stat{
  uint64_t input_port_incoming_pkts;
  uint64_t input_port_outgoing_pkts;
  uint64_t input_port_dropped_pkts;

  uint64_t output_port_incoming_pkts;
  uint64_t output_port_outgoing_pkts;
  uint64_t output_port_dropped_pkts;


  uint64_t control_port_incoming_pkts;
  uint64_t control_port_outgoing_pkts;
  uint64_t control_port_dropped_pkts;

  uint64_t active_flows;
  uint64_t inactive_flows;

  uint64_t migration_index;
  uint64_t migration_target_runtime_id;
  uint64_t migration_qouta;
  uint64_t average_flow_migration_completion_time;
  uint64_t toal_flow_migration_completion_time;
  uint64_t successful_migration;

  storage_stat* array;
  uint64_t array_size;

  runtime_stat(uint64_t size) :
    input_port_incoming_pkts(0),
    input_port_outgoing_pkts(0),
    input_port_dropped_pkts(0),
    output_port_incoming_pkts(0),
    output_port_outgoing_pkts(0),
    output_port_dropped_pkts(0),
    control_port_incoming_pkts(0),
    control_port_outgoing_pkts(0),
    control_port_dropped_pkts(0),
    active_flows(0),
    inactive_flows(0),
    migration_index(0),
    migration_target_runtime_id(0),
    migration_qouta(0),
    average_flow_migration_completion_time(0),
    toal_flow_migration_completion_time(0),
    successful_migration(0){
    if(size>0){
      array = static_cast<storage_stat*>(mem_alloc(sizeof(storage_stat)*size));
    }
    else{
      array=nullptr;
    }
    array_size = size;
  }

  ~runtime_stat(){
    if(array!=nullptr){
      mem_free(array);
    }
  }
};

enum class rpc_operation{
  add_input_runtime,
  add_output_runtime,
  delete_input_runtime,
  delete_output_runtime,
  set_migration_target,
  add_replica,
  add_storage,
  remove_replica,
  remove_storage,
  get_stats
};

struct llring_item{
  rpc_operation op_code;
  runtime_config rt_config;
  uint64_t migration_qouta;
  runtime_stat stat;

  llring_item(rpc_operation code,
              runtime_config config,
              uint64_t qouta,
              uint64_t replica_num) :
                op_code(code),
                rt_config(config),
                migration_qouta(qouta),
                stat(replica_num){}
};

#endif
