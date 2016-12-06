#include <iostream>
#include <string>

#include "nfaflags.h"

using namespace std;

DEFINE_bool(boolean_flag, true, "a test boolean flag");

static bool ValidatePort(const char* flagname, const char* str_arg) {
  string value(str_arg);
  if (value == "wtf")   // value is ok
     return true;
   printf("Invalid value for --%s: %s\n", flagname, value.c_str());
   return false;
}
DEFINE_string(string_flag, "a,b,c", "comma spearated string flag");
DEFINE_validator(string_flag, &ValidatePort);
