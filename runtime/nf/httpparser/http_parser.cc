#include "http_parser.h"

#include "../base/network_function_register.h"

bool registered_http_parser =
    static_nf_register::get_register().register_nf<http_parser, http_parser_fs>("http_parser", 4);
