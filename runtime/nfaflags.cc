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

DEFINE_string(rpc_ip, "127.0.0.1", "The IP address that the RPC server listens on.");

DEFINE_int32(rpc_port, 10240, "The port that the RPC server listens on");
