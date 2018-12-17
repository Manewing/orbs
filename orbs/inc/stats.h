#ifndef STATS_HH
#define STATS_HH

#include <stdio.h>

// sampling rate (number of iterations per sample)
#define STATS_SAMPLE_RATE 256 // TODO make configurable

// count of samples that are buffered
#define STATS_BUFFER_SIZE 2048 // TODO make configurable

// forward definition of map
struct map_t;

typedef struct stats_t {

  // sample index
  int sample_idx;

  // current buffer index
  int buffer_idx;

  // current amount of food on the map
  int food[STATS_BUFFER_SIZE];

  // current number of orbs on the map
  int orbs[STATS_BUFFER_SIZE];
  FILE *orbs_f;

  // average life time of orbs
  int avg_life_time[STATS_BUFFER_SIZE];
  FILE *avg_life_time_f;

  // average instruction usage of orbs (in percent * 1000)
  int avg_instr_usage[STATS_BUFFER_SIZE];
  FILE *avg_instr_usage_f;

  // average instruction usage of orbs per instruction (in percent * 1000)
  int avg_instr_usage_per[STATS_BUFFER_SIZE][16];

} stats_t;

stats_t *create_stats(const char *output_dir);

void update_stats(stats_t *stats, struct map_t *map);

void flush_stats(stats_t *stats);

void free_stats(stats_t *stats);

#endif // #ifndef STATS_HH
