#include <argp.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "config.h"
#include "defines.h"
#include "map.h"
#include "orb.h"
#include "shell.h"

#define Hz(x) (1000000 / (x))

const char *argp_program_version = "ORBs 1.0";
const char *argp_program_bug_address = "todo";
static char doc[] = "ORBs cell program simulation.";
static char args_doc[] = "";
static struct argp_option options[] = {
    {"skip", 's', "NUM", 0, "Skip first NUM iterations", 0},
    {"exit", 'e', "NUM", 0, "Exit after NUM iterations", 0},
    {"herz", 'h', "HZ", 0, "Update rate in herz", 0},
    {"config", 'f', "FILE", 0, "Configuration FILE to load", 0},
    {"configure", 'c', "PARAM=VALUE", 0, "Configure a specific parameter", 0},
    {"config-info", 513, NULL, 0,
     "Prints information about all configurable parameters.", 0},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  config_reader_t *cfg_reader = state->input;

  switch (key) {
  case 'c':
    if (read_config_line(cfg_reader, arg) != 0)
      exit(-1);
    break;
  case 'f':
    if (read_config_file(cfg_reader, arg) != 0)
      exit(-1);
    break;
  case 'h':
    read_config_value(cfg_reader, "global_config.herz", arg);
    break;
  case 's':
    read_config_value(cfg_reader, "global_config.skip", arg);
    break;
  case 'e':
    read_config_value(cfg_reader, "global_config.exit", arg);
    break;
  case 513:
    print_config_options(cfg_reader);
    exit(0);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

typedef enum { ST_EXIT, ST_RUNNING, ST_PAUSED } orbs_state_t;
static orbs_state_t orbs_state, orbs_request;

static pthread_mutex_t orbs_state_mtx;
static pthread_cond_t orbs_state_cond;

static void orbs_state_init(void) {
  orbs_state = ST_RUNNING;
  orbs_request = ST_RUNNING;
  pthread_cond_init(&orbs_state_cond, NULL);
  pthread_mutex_init(&orbs_state_mtx, NULL);
}

static void orbs_state_change(orbs_state_t state) {
  pthread_mutex_lock(&orbs_state_mtx);
  orbs_request = state;
  pthread_cond_signal(&orbs_state_cond);
  pthread_mutex_unlock(&orbs_state_mtx);
  while (orbs_state != state)
    ;
}

static void orbs_state_handle(void) {
  pthread_mutex_lock(&orbs_state_mtx);
  switch (orbs_request) {
  case ST_PAUSED:
    orbs_state = ST_PAUSED;
    while (orbs_request == ST_PAUSED) {
      pthread_cond_wait(&orbs_state_cond, &orbs_state_mtx);
    }
    orbs_state = ST_RUNNING;
    break;
  default:
    orbs_state = orbs_request;
    break;
  }
  pthread_mutex_unlock(&orbs_state_mtx);
}

static void *input(void *ptr);

map_t *map = NULL;

void handle_exit(int sig) {
  free_map(map);
  exit(sig);
}

void check_population(map_t const *map, unsigned long iteration) {
  // check if population died out
  if (map->orbs.size) {
    return;
  }

  printf("Population died out @iteration = %lu\n", iteration);
  orbs_state = ST_EXIT;
  handle_exit(0);
}

int main(int argc, char *argv[]) {
  // parse command line arguments
  argp_parse(&argp, argc, argv, 0, 0, &global_config_reader);

  // init seed
  srand(global_config.seed);

  // create map
  map = create_map();
  signal(SIGINT, handle_exit);

  // create orbs
  for (int l = 0; l < global_config.orb_count; l++) {
    orb_t *orb = create_orb();
    reset_orb_genes(orb);
    map_add_orb(map, orb);
  }

  // setup state handling
  orbs_state_init();

  // check that number of skip iterations is not more than exit
  if (global_config.skip > global_config.exit) {
    log_error("main", "Number #skip cannot be more than #exit\n");
    handle_exit(0);
  }

  // skip given amount of iterations
  for (unsigned long l = 0; l < global_config.skip; l++) {
    update_map(map);
    check_population(map, l);
  }

  // start input thread
  pthread_t input_thread;
  pthread_create(&input_thread, NULL, input, NULL);

  // main loop
  unsigned long iterations = global_config.exit - global_config.skip;
  for (unsigned long l = 0; l < iterations && orbs_state != ST_EXIT; l++) {
    usleep(Hz(global_config.herz));

    update_map(map);
    draw_map(map);
    check_population(map, l);

    // check for request and handle it
    if (orbs_request != ST_RUNNING) {
      orbs_state_handle();
    }
  }

  handle_exit(0);
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

static orb_t *get_orb_idx(int idx) {
  node_t *node = map->orbs.head;
  while (node) {
    if (idx-- <= 0)
      return node->data;
    node = node->next;
  }
  return NULL;
}

static void *input(void *ptr) {
  (void)ptr;

  // enable direct input
  enable_direct_input();

  unsigned int hl_idx = 0, hl_idx_l;
  orb_t *orb;
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
    case 'a':
      hl_idx_l = hl_idx;
      hl_idx = hl_idx == 0 ? 0 : (hl_idx - 1);
      orb = get_orb_idx(hl_idx_l);
      if (orb)
        orb_feed(orb, 0);
      orb = get_orb_idx(hl_idx);
      if (orb)
        orb->body = 'X';
      printf("orb%d", orb->id);
      break;
    case 'd':
      hl_idx_l = hl_idx;
      hl_idx = ++hl_idx > map->orbs.size ? map->orbs.size - 1 : hl_idx;
      orb = get_orb_idx(hl_idx_l);
      if (orb)
        orb_feed(orb, 0);
      orb = get_orb_idx(hl_idx);
      if (orb)
        orb->body = 'X';
      printf("orb%d", orb->id);
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
