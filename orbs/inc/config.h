#ifndef CONFIG_HH
#define CONFIG_HH

#include "config_reader.h"
#include <stdio.h>

typedef struct {
  /// Number of iterations to skip
  unsigned long skip;

  /// Number of iterations to exit after
  unsigned long exit;

  /// Seed for the scenario
  int seed;

  /// Current frequency for simulation
  int herz;

  /// Path to the output directory for statistics
  const char stats[FILENAME_MAX];

} global_config_t;

extern global_config_t global_config;
extern config_reader_t global_config_reader;

#endif // #ifndef CONFIG_HH
