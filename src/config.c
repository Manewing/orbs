#include "config.h"
#include "defines.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define log_config_warn(fmt, ...) log_warn("CONFIG", fmt, ##__VA_ARGS__)
#define log_config_error(fmt, ...) log_error("CONFIG", fmt, ##__VA_ARGS__)

config_t global_config = {
  .skip               =      0,
  .seed               =      0,
  .herz               =    201,
  .orb_count          =     80,
  .orb_score          =   1500,
  .orb_ttl            = 0xffffff,
  .orb_mutation       =  10000,
  .orb_bodies         = {  '.',  'o',  'O' },
  .orb_scores         = { 2000, 4000, 8000 },
  .food_rate          =     30,
  .food_types         = {  '+',  '#' },
  .food_scores        = { 1000, 4000 }
};

#define __config_elem(cfg_t, cfg, elem, elem_t, doc) \
  { #cfg "." #elem, elem_t, doc, (char*)&cfg + offsetof(cfg_t, elem) }

struct config_elem {
  const char* name;
  const char* type;
  const char* doc;
  void*       ptr;
};

static struct config_elem config_elems[] = {
  __config_elem(config_t, global_config, seed, "%d", "Seed for srand (default = 0)"),
  __config_elem(config_t, global_config, orb_count, "%d", "Initial count of orbs"),
  __config_elem(config_t, global_config, orb_score, "%d", "Initial score of orb"),
  __config_elem(config_t, global_config, orb_ttl, "%d", "(Max) Time to live of orb"),
  __config_elem(config_t, global_config, orb_mutation, "%d", "Mutation rate of orbs P(mutate) = 1 / orb_mutation"),
  __config_elem(config_t, global_config, orb_scores[0], "%d", "Min. score for orb type '.'"),
  __config_elem(config_t, global_config, orb_scores[1], "%d", "Min. score for orb type 'o'"),
  __config_elem(config_t, global_config, orb_scores[2], "%d", "Min. score for orb type 'O'"),
  __config_elem(config_t, global_config, food_rate, "%d", "Food spawn rate P(food) = 1 / food_rate"),
  __config_elem(config_t, global_config, food_scores[0], "%d", "Score of food '+'"),
  __config_elem(config_t, global_config, food_scores[1], "%d", "Score of food '#'")
};

static int config_elems_count(void) {
  return sizeof(config_elems) / sizeof(struct config_elem);
}

#undef __config_elem

static void replace_char(char* str, char find, char replace) {
  int l;
  for (l = 0; l < strlen(str); l++) {
    if (str[l] == find)
      str[l] = replace;
  }
}

int read_config_line(const char* line) {
  int n, l;
  char buffer[256];

  char* linedup = strdup(line);
  replace_char(linedup, '=', ' ');
  replace_char(linedup, ',', ' ');

  // read config element name
  if (sscanf(linedup, "%s%n", buffer, &n) != 1) {
    log_config_error("cannot read parameter from line: %s\n", line);
    free(linedup);
    return -1;
  }

  for (l = 0; l < config_elems_count(); l++) {
    if (strcmp(buffer, config_elems[l].name) == 0) {
      if (sscanf(linedup + n, config_elems[l].type, config_elems[l].ptr) != 1) {
        log_config_error("cannot read parameter value from line: %s\n", line);
        free(linedup);
        return -1;
      }
      free(linedup);
      return 0;
    }
  }

  log_config_error("unkown config parameter: '%s'\n", buffer);
  return -1;
}

int read_config_file(const char* file) {
  char* line = NULL;
  size_t linecap, linecount = 0;

  FILE* cfg = fopen(file, "r");

  if (cfg == NULL) {
    log_config_error("could not open file %s for read\n", file);
    return -1;
  }

  while (getline(&line, &linecap, cfg) > 0) {
    linecount++;

    // skip empty lines and comments
    if (strcmp(line, "\n") == 0 || line[0] == '#')
      continue;

    if (read_config_line(line) != 0) {
      log_config_error("syntax error in line %ld of file %s\n", linecount, file);
      return -1;
    }

  }

  fclose(cfg);

  return 0;
}

void print_config_options(void) {
  printf("Configuration information:\n");
  printf("%-30s %s\n", "# name", "# type    # doc");

  int l;
  for (l = 0; l < config_elems_count(); l++) {
    printf("  %-30s %s        %s\n",
        config_elems[l].name,
        config_elems[l].type,
        config_elems[l].doc);
  }

  printf("\nTypes correspond to format parameters of 'scanf'.\n");
}
