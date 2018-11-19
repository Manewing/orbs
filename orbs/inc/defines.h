#ifndef DEFINES_HH
#define DEFINES_HH

#include <stdio.h>

#define __log(file, msg, module, fmt, ...) \
  fprintf(file, "[" msg "] "  module ": " fmt, ##__VA_ARGS__)

#define log_warn(module, fmt, ...) __log(stderr, "WARN", module, fmt, ##__VA_ARGS__)
#define log_error(module, fmt, ...) __log(stderr, "ERROR", module, fmt, ##__VA_ARGS__)

#endif // #ifndef DEFINES_HH
