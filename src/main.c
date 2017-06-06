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
#include "shell.h"

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
  { "foodscore-0", 513, "SCORE", 0, "Sets SCORE of food type 0"},
  { "foodscore-1", 514, "SCORE", 0, "Sets SCORE of food type 1"},
  {0}
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  config_t* config = state->input;

  int type = 0;

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
    case 514:
      type = 1;
    case 513:
      {
        config->food_scores[type] = atoi(arg);
      }
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };


typedef enum { ST_EXIT, ST_RUNNING, ST_PAUSED } orbs_state_t;
static orbs_state_t orbs_state, orbs_request;

static void orbs_state_change(orbs_state_t state) {
  orbs_request = state;
  while (orbs_state != state);
}

static void* input(void* ptr);

map_t* map = NULL;

int main(int argc, char* argv[]) {
  int l;

  // parse command line arguments
  argp_parse(&argp, argc, argv, 0, 0, &global_config);

  // init seed
  srand(global_config.seed);

  // create map
  map = create_map();

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
  orbs_state = ST_RUNNING;
  orbs_request = ST_RUNNING;
  while (orbs_state != ST_EXIT) {
    update_map(map);
    draw_map(map);

    // check if population died out
    if (!map->orbs.size) {
      orbs_state = ST_EXIT;
      printf("Population died out...\n");
    }

    if (orbs_request == ST_EXIT)
      orbs_state = ST_EXIT;
    else if (orbs_request == ST_PAUSED) {
      orbs_state = ST_PAUSED;
      while (orbs_request == ST_PAUSED);
      orbs_state = ST_RUNNING;
    }

    usleep(Hz(global_config.herz));
  }

  pthread_join(input_thread, NULL);

  free_map(map);
  return 0;
}

struct termios t0, t1;

static void enable_direct_input(void) {
  tcgetattr(STDIN_FILENO, &t0);
  t1 = t0;
  t1.c_lflag &= ~(ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &t1);
}

static void disable_direct_input(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &t0);
}

static void* input(void* ptr) {
  (void)ptr;

  // enable direct input
  enable_direct_input();

  while (orbs_state != ST_EXIT) {
    int c = getc(stdin);
    switch (c) {
      case 'q':
        orbs_request = ST_EXIT;
        break;
      case 'w':
        global_config.herz += 10;
        break;
      case 's':
        global_config.herz -= 10;
        global_config.herz = global_config.herz < 0 ? 1 : global_config.herz;
        break;
      case 'p':
        disable_direct_input();
        orbs_state_change(ST_PAUSED);
        orbs_shell();
        orbs_state_change(ST_RUNNING);
        enable_direct_input();
        break;
      default:
        break;
    }
  }

  // disable direct input
  disable_direct_input();

  return NULL;
}
