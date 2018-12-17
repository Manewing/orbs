#ifndef CONFIG_HH
#define CONFIG_HH

#include <stdio.h>

typedef struct {
  int skip;
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

} config_t;

extern config_t global_config;

int read_config_line(const char *line);
int read_config_file(const char *file);

void print_config_options(void);

#endif // #ifndef CONFIG_HH
