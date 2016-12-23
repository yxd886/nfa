#include "flow_monitor.h"

#include "../base/network_function_register.h"

bool registered_flow_monitor =
    static_nf_register::get_register().register_nf<flow_monitor, flow_monitor_fs>("flow_monitor", 2);
