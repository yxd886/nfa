#include <iostream>
#include <string>

#include "nfaflags.h"

using namespace std;

DEFINE_bool(boolean_flag, true, "a test boolean flag");

DEFINE_string(string_flag, "a,b,c", "comma spearated string flag");

DEFINE_int32(temp_core, 5, "Temporary lcore binding.");

DEFINE_string(input_port, "", "Name of the input port");

DEFINE_string(output_port, "", "Name of the output port");

DEFINE_int32(rpc_timeout, 10, "RPC timeout in milliseconds, default value is 10ms");

DEFINE_int32(runtime_id, 0, "The ID of the runtime.");

DEFINE_string(input_port_mac, "11:11:11:11:11:11", "The mac address of the input port.");

DEFINE_string(output_port_mac, "22:22:22:22:22:22", "The mac address of the output port.");

DEFINE_string(control_port_mac, "33:33:33:33:33:33", "The mac address of the control port");

DEFINE_string(rpc_ip, "127.0.0.1", "The IP address that the RPC server listens on.");

DEFINE_int32(rpc_port, 10240, "The port that the RPC server listens on");
