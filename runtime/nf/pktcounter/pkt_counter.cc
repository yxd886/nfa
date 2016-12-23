#include "pkt_counter.h"
#include "../base/network_function_register.h"

bool registered = network_function_register::register_nf<pkt_counter, pkt_counter_fs>("pkt_counter", 1);
