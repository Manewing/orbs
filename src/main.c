#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "map.h"
#include "orb.h"
#include "config.h"

#define Hz(x) (1000000 / (x))

const char* argp_program_version = "ORBs 1.0";
const char* argp_program_bug_address = "todo";
static char doc[] = "ORBs cell program simulation.";
static char args_doc[] = "";
static struct argp_option options[] = {
  { "seed", 'r', "SEED", 0, "Seed for srand (default = 0)" },
  { "skip", 's', "NUM", 0, "Skip first NUM iterations" },
  { "orbs", 'o', "NUM", 0, "Number of orbs in initial population" },
  { "mutate", 'm', "NUM", 0, "Rate at which mutations take place P(mutate) = (1 / NUM" },
  { "food", 'f', "NUM", 0, "Rate at which food spawns P(food) = (1 / NUM)" }
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  config_t* config = state->input;

  switch (key) {
    case 'r':
      config->seed = atoi(arg);
      break;
    case 's':
      config->skip = atoi(arg);
      break;
    case 'o':
      config->orb_count = atoi(arg);
      break;
    case 'm':
      config->orb_mutation = atoi(arg);
      break;
    case 'f':
      config->food_rate = atoi(arg);
      break;
    /*case ARGP_KEY_ARG:
      return 0;
    case ARGP_KEY_END:
      return 0;*/
    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char* argv[]) {
  int l;

  // parse command line arguments
  argp_parse(&argp, argc, argv, 0, 0, &global_config);

  // init seed
  srand(global_config.seed);

  // create map
  map_t* map = create_map();

  // create orbs
  for (l = 0; l < global_config.orb_count; l++) {
    orb_t* orb = create_orb();
    reset_orb_genes(orb);
    map_add_orb(map, orb);
  }

  // skip given amount of iterations
  for (l = 0; l < global_config.skip; l++) {
    update_map(map);
  }

  // main loop
  while(map->orbs.size) {
    update_map(map);
    draw_map(map);
    usleep(Hz(200));
  }

  printf("Population died out...\n");

  free_map(map);
  return 0;
}
