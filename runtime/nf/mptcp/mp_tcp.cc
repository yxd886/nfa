#include "../mptcp/mp_tcp.h"

#include "../base/network_function_register.h"

bool registered_mp_tcp =
    static_nf_register::get_register().register_nf<mp_tcp, mp_tcp_fs>("mp_tcp", 5);

int32_t mp_tcp::runtime_id=-1;
uint32_t mp_tcp::target_no=0;
