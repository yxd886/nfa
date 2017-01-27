#include "ids.h"

#include "../base/network_function_register.h"

bool registered_ids =
    static_nf_register::get_register().register_nf<ids, ids_fs>("ids", 5);
