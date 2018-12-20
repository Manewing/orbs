#ifndef CONFIG_HH
#define CONFIG_HH

#include "config_reader.h"
#include <stdio.h>

typedef struct {
  unsigned long skip;
  unsigned long exit;

  int seed;
  int herz;

  int orb_count;
  int orb_score;
  int orb_ttl;
  int orb_mutation;
  char orb_bodies[3];
  int orb_scores[3];

  int food_rate;
  char food_types[2];
  int food_scores[2];

  const char stats_output[FILENAME_MAX];

} global_config_t;

extern global_config_t global_config;
extern config_reader_t global_config_reader;

#endif // #ifndef CONFIG_HH
