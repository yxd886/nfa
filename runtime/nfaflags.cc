#include <iostream>
#include <string>

#include "nfaflags.h"

using namespace std;

DEFINE_bool(boolean_flag, true, "a test boolean flag");

DEFINE_string(string_flag, "a,b,c", "comma spearated string flag");

DEFINE_int32(temp_core, 1, "Temporary lcore binding.");

DEFINE_string(input_port, "", "Name of the input port");

DEFINE_string(output_port, "", "Name of the output port");
