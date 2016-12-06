#ifndef NFAFLAGS_H
#define NFAFLAGS_H

#include <gflags/gflags.h>

DECLARE_bool(boolean_flag, true, "a test boolean flag");
DECLARE_string(string_flag, "a,b,c", "comma spearated string flag");

#endif
