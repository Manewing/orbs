#include "stats.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "map.h"
#include "orb.h"

stats_t *create_stats(const char *output_dir) {
  stats_t *stats = (stats_t *)malloc(sizeof(stats_t));
  assert(stats != NULL);
  memset(stats, 0, sizeof(stats_t));

  // buffer for filenames
  char buffer[FILENAME_MAX];
  size_t output_dir_size = strlen(output_dir);

  // TODO handle case when file could not be opened..
  // open files
  memcpy(buffer, output_dir, output_dir_size + 1);
  strcat(buffer, "/orbs.stats");
  stats->orbs_f = fopen(buffer, "wb");

  memcpy(buffer, output_dir, output_dir_size + 1);
  strcat(buffer, "/avg_life_time.stats");
  stats->avg_life_time_f = fopen(buffer, "wb");

  memcpy(buffer, output_dir, output_dir_size + 1);
  strcat(buffer, "/avg_instr_usage.stats");
  stats->avg_instr_usage_f = fopen(buffer, "wb");

  stats->sample_idx = STATS_SAMPLE_RATE;
  return stats;
}

static int get_avg_life_time(list_t const *orbs) {
  int avg_life_time = 0;

  if (orbs->size == 0)
    return avg_life_time;

  node_t const *node = orbs->head;
  while (node) {
    orb_t const *orb = node->data;
    avg_life_time += orb->lifetime;
    node = node->next;
  }

  return avg_life_time / orbs->size;
}

static int get_avg_instr_usage(list_t const *orbs) {
  float avg_instr_usage = 0;

  if (orbs->size == 0)
    return avg_instr_usage;

  node_t const *node = orbs->head;
  while (node) {
    orb_t const *orb = node->data;
    avg_instr_usage += get_orb_instr_usage(orb);
    node = node->next;
  }

  return (int)(avg_instr_usage * 1000.0 / orbs->size);
}

void update_stats(stats_t *stats, struct map_t *map) {
  // check if we need to add another sample
  if (++stats->sample_idx <= STATS_SAMPLE_RATE) {
    return;
  }

  // update orb count
  stats->orbs[stats->buffer_idx] = map->orbs.size;

  // update average lifetime
  stats->avg_life_time[stats->buffer_idx] = get_avg_life_time(&map->orbs);

  // update average instruction usage
  stats->avg_instr_usage[stats->buffer_idx] = get_avg_instr_usage(&map->orbs);

  stats->sample_idx = 0;
  stats->buffer_idx++;

  if (stats->buffer_idx >= STATS_BUFFER_SIZE) {
    flush_stats(stats);
  }
}

void flush_stats(stats_t *stats) {
  if (stats->buffer_idx == 0) {
    return;
  }

  // write data to file
  fwrite(stats->orbs, sizeof(int), STATS_BUFFER_SIZE, stats->orbs_f);
  fwrite(stats->avg_life_time, sizeof(int), STATS_BUFFER_SIZE,
         stats->avg_life_time_f);
  fwrite(stats->avg_instr_usage, sizeof(int), STATS_BUFFER_SIZE,
         stats->avg_instr_usage_f);

  stats->buffer_idx = 0;
}

void free_stats(stats_t *stats) {
  assert(stats != NULL);

  // flush statistics
  flush_stats(stats);

  // close files
  fclose(stats->orbs_f);
  fclose(stats->avg_life_time_f);
  fclose(stats->avg_instr_usage_f);

  free(stats);
}
