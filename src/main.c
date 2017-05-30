#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

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
  { "herz", 'h', "HZ", 0, "Update rate in herz" },
  { "orbs", 'o', "ORBS", 0, "Number of orbs in initial population" },
  { "mutate", 'm', "NUM", 0, "Rate at which mutations take place P(mutate) = (1 / NUM" },
  { "food", 'f', "NUM", 0, "Rate at which food spawns P(food) = (1 / NUM)" },
  {0}
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
    case 'h':
      config->herz = atoi(arg);
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
    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int quit = 0;
void* input(void* ptr);

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

  // start input thread
  pthread_t input_thread;
  pthread_create(&input_thread, NULL, input, NULL);

  // main loop
  while(!quit) {
    update_map(map);
    draw_map(map);
    if (!map->orbs.size)
      quit = 1;
    usleep(Hz(global_config.herz));
  }

  pthread_join(input_thread, NULL);

  if (!map->orbs.size)
    printf("Population died out...\n");

  free_map(map);
  return 0;
}

void* input(void* ptr) {
  (void)ptr;
  struct termios t0, t1;
  tcgetattr(STDIN_FILENO, &t0);
  t1 = t0;
  t1.c_lflag &= ~(ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &t1);

  while (!quit) {
    int c = getc(stdin);
    switch (c) {
      case 'q':
        quit = 1;
        break;
      case 'w':
        global_config.herz += 10;
        break;
      case 's':
        global_config.herz -= 10;
        global_config.herz = global_config.herz < 0 ? 1 : global_config.herz;
        break;
      default:
        break;
    }
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &t0);

  return NULL;
}
