#ifndef NFAFLAGS_H
#define NFAFLAGS_H

#include <gflags/gflags.h>

DECLARE_bool(boolean_flag);
DECLARE_string(string_flag);
DECLARE_int32(temp_core);
DECLARE_string(input_port);
DECLARE_string(output_port);
DECLARE_int32(rpc_timeout);
DECLARE_string(rpc_ip);
DECLARE_int32(rpc_port);

#endif
