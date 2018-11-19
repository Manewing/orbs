#include "shell.h"

#include "map.h"
#include "orb.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#define FAIL(cond, fmt, ...) if ((cond)) { printf(fmt, ##__VA_ARGS__); return -1; }

#define ARG_CNT_FAIL(idx) FAIL(args[idx] == NULL, "not enough arguments\n")
#define ORB_SEL_FAIL      FAIL(current_orb == NULL, "no orb selected\n")
#define GET_ORB_FAIL(orb, str) \
  orb = str2orb(str); FAIL(orb == NULL, "no such orb: %s\n", str)

extern map_t* map;

static char* orbs_shell_getline(char* pref) {
  char* line = readline(pref);
  if (!line) {
    return NULL;
  }
  add_history(line);
  return line;
}

static char** orbs_shell_split_args(char* line) {
  int bufsize = 64, pos = 0;
  char* arg;
  char** args = malloc(sizeof(char*) * bufsize);

  arg = strtok(line, " \n\r\a\t");
  while (arg != NULL) {

    args[pos++] = arg;

    if (pos >= bufsize) {
      bufsize <<= 2;
      args = realloc(args, sizeof(char*) * bufsize);
    }

    arg = strtok(NULL, " \n\r\a\t");
  }

  args[pos] = NULL;

  return args;
}

static orb_t* get_orb(int id) {
  node_t* node = map->orbs.head;
  while (node) {
    orb_t* orb = node->data;

    if (orb->id == id)
      return orb;

    node = node->next;
  }
  return NULL;
}

static orb_t* str2orb(char* str) {
  if (strlen(str) > 3) {
    int id = atoi(str + 3);
    return get_orb(id);
  }

  return NULL;
}

static void print_orb_status(orb_t* orb) {
  printf("%8p  ->  map/orb%d/ @ [%2d, %2d]\n", orb, orb->id, orb->x, orb->y);
  printf("  [Score]  : %d\n", orb->score);
  printf("  [TTL]    : %d\n", orb->ttl);
  printf("  [Status] : 0x%02x\n", orb->status);
  printf("  [Regs]   : r0: 0x%02x, r1: 0x%02x, r2: 0x%02x, r3: 0x%02x\n",
      orb->regs[0], orb->regs[1], orb->regs[2], orb->regs[3]);
  printf("  [LR]     : 0x%02x\n", orb->lr);
  printf("  [Idx]    : 0x%02x\n", orb->idx);
  printf("  [IMU]    : %.2f %%\n", get_orb_instr_usage(orb));
}

static void print_map_status(map_t* map) {
  printf("%8p  ->  map/\n", &map->orbs);
  printf("  [Iter]   : 0x%lx\n", map->iteration);
  printf("  [Orbs]   : %ld\n", map->orbs.size);
}

static orb_t * current_orb = NULL;

static int print_orb_disas_instr(orb_t const* orb, int idx) {
  int n;
  char buffer[64];

  // disassemble instruction
  n = orb_disas(orb, idx, buffer);

  if (strcmp(buffer, "nop") == 0)
    printf("\e[37m");

  printf("%*s [ %6.2f%% ]\r", 60, " ", 100.0*orb->trace[idx]/orb->trace_count);
  printf("%s[0x%02x]: 0x%02x %s\n\e[39m", orb->idx == idx ? " -> " : "    ",
      idx, orb->genes[idx], buffer);

  return n;
}

static void do_orb_disas(orb_t const* orb, int nodeadcode) {
  printf("%p -> genes @ map/orb%d/\n", orb, orb->id);

  int idx = 0;
  while (idx < ORB_GENE_SIZE) {

    int dead = (float)orb->trace[idx]/orb->trace_count < .0001;
    if(nodeadcode && dead) {
      idx++;
      continue;
    }

    idx += print_orb_disas_instr(orb, idx);
  }
}

static int orbs_cmd_disas(char** args) {
  ORB_SEL_FAIL;

  int nodeadcode = 0;
  if (args[1] != NULL && strcmp(args[1], "ndc") == 0)
    nodeadcode = 1;

  do_orb_disas(current_orb, nodeadcode);
  return 1;
}

static int orbs_cmd_p(char** args) {

  orb_t* orb = NULL;

  if (args[1] == NULL && current_orb == NULL) {
    print_map_status(map);
    return 1;
  } else if (args[1] != NULL && strcmp(args[1], "*") == 0) {
    FAIL(current_orb != NULL, "option '*' not allowed for orb\n");
    node_t* node = map->orbs.head;
    while (node) {
      orb_t* orb = node->data;
      print_orb_status(orb);
      node = node->next;
    }
    return 1;
  } else if (args[1] != NULL) {
    GET_ORB_FAIL(orb, args[1]);
  } else {
    ORB_SEL_FAIL;
    orb = current_orb;
  }

  print_orb_status(orb);

  return 1;
}

static int orbs_cmd_s(char** args) {
  ORB_SEL_FAIL;
  int id = current_orb->id;

  // do one step
  update_map(map);

  current_orb = get_orb(id);
  if (current_orb == NULL) {
    printf("orb%d died...\n", id);
    return 1;
  }

  print_orb_disas_instr(current_orb, current_orb->idx);

  return 1;
}

static int orbs_cmd_highlight(char** args) {

  int idx = 0;
  orb_t* orb = NULL;
  static int hl_orbs[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

  if (args[1] != NULL) {
    if (strcmp(args[1], "rm") == 0) {
      ARG_CNT_FAIL(2);

      idx = atoi(args[2]);
      FAIL(idx < 0 || idx > 9, "index out of range: %d", idx);

      // if orb is still alive un highlight (by feeding nothing)
      orb = get_orb(hl_orbs[idx]);
      if (orb) {
        orb->highlight = 0;
        orb_feed(orb, 0);
      }

      hl_orbs[idx] = -1;
      return 1;
    } else if (strcmp(args[1], "info") == 0) {
      for (; idx < 10; idx++) {
        if (hl_orbs[idx] != -1)
          printf("highlight %d: orb%d\n", idx, hl_orbs[idx]);
      }
      return 1;
    } else {
      GET_ORB_FAIL(orb, args[1]);
    }
  } else {
    ORB_SEL_FAIL;
    orb = current_orb;
  }

  while (hl_orbs[idx] != -1 && idx++ < 10);
  FAIL(idx > 9, "cannot highlight more than 10 orbs\n");

  hl_orbs[idx] = orb->id;
  orb->body = '0' + idx;
  orb->highlight = 1;
  printf("highlight %d: orb%d\n", idx, hl_orbs[idx]);

  return 1;
}

static int orbs_cmd_ls_map(void) {
  printf("%8p  ->  map/\n", &map->orbs);

  int idx = 0;
  node_t* node = map->orbs.head;
  while (node) {
    orb_t* orb = node->data;
    printf("    %8p  ->  orb%d\n", orb, orb->id);
    node = node->next;
  }

  printf("total %ld\n", map->orbs.size);
  return 1;
}

static int orbs_cmd_ls_orb(orb_t* orb) {
  int l, idx = orb->idx - 4;

  printf("%8p  ->  map/orb%d/ @ [%2d, %2d]\n", orb, orb->id, orb->x, orb->y);

  for (l = 0; l < 9; l++) {
    int _idx = (idx+l) & 0xff;

    if (_idx == orb->idx)
      printf(" -> ");
    else
      printf("    ");

    print_orb_disas_instr(orb, _idx);
  }

  return 1;
}

static int orbs_cmd_ls(char** args) {

  orb_t* orb = NULL;

  if (args[1] == NULL && current_orb == NULL) {
    // map is selected, list all orbs on map
    return orbs_cmd_ls_map();
  } else if (args[1] != NULL && !strcmp(args[1], "*") && current_orb == NULL) {
    // map is selected, command with wildcard, call ls on all orbs
    node_t* node = map->orbs.head;
    while (node) {
      orb_t* orb = node->data;
      orbs_cmd_ls_orb(orb);
      node = node->next;
    }
    return 1;
  } else if (args[1] != NULL && !strcmp(args[1], "*") && current_orb != NULL) {
    // an orb is selected call, command with wildcard, like disas
    do_orb_disas(current_orb, 0);
    return 1;
  } else if (args[1] != NULL) {
    // map is selected, get selected orb from argument
    GET_ORB_FAIL(orb, args[1]);
  } else {
    // orb is selected, run command on selected orb
    ORB_SEL_FAIL;
    orb = current_orb;
  }

  return orbs_cmd_ls_orb(orb);
}

static int orbs_cmd_cd(char** args) {
  if (args[1] == NULL)
    current_orb = NULL;
  else if (strcmp(args[1], "..") == 0) {
    current_orb = NULL;
  } else {
    orb_t* orb;
    GET_ORB_FAIL(orb, args[1]);
    current_orb = orb;
  }

  return 1;
}

static int orbs_cmd_c(char** args) {
  return 0;
}

struct orbs_command {
  const char* cmd;
  const char* help;
  int (*command_func)(char**);
};

static int orbs_cmd_help(char** args);

static struct orbs_command orbs_commands[] = {
  { "l", "lists (sub) entities of entity, disassembly if entity is an orb", &orbs_cmd_ls},
  { "cd", "change/select entity (like cd in shell)", &orbs_cmd_cd },
  { "p", "print status of entity", &orbs_cmd_p },
  { "hl", "highlight a given orb, rm for removing, info for listing highlight", &orbs_cmd_highlight},
  { "s", "single step currently selected orb", &orbs_cmd_s },
  { "disas", "print disassembly of genes of selected orb, use 'disas ndc' to skip deadcode", &orbs_cmd_disas },
  { "c", "continue simulation", &orbs_cmd_c },
  { "help", "print this help screen", &orbs_cmd_help }
};

static int orbs_command_count(void) {
  return sizeof(orbs_commands) / sizeof(struct orbs_command);
}

static int orbs_cmd_help(char** args) {
  int l;
  printf("Commands:\n");
  for (l = 0; l < orbs_command_count(); l++) {
    printf("%10s -- %s\n", orbs_commands[l].cmd, orbs_commands[l].help);
  }
  return 1;
}

static int orbs_shell_execute(char** args) {
  if (args[0] == NULL)
    return 1;

  int l;
  for (l = 0; l < orbs_command_count(); l++) {
    if (strcmp(args[0], orbs_commands[l].cmd) == 0)
      return orbs_commands[l].command_func(args);
  }

  printf("unknown command: %s\n", args[0]);

  return 1;
}

void orbs_shell(void) {
  int status;
  char* line;
  char** args;

  char buffer[256];

  // clear screen
  printf("\033[H\033[J");

  do {
    int n = sprintf(buffer, "orbs/map/");
    if (current_orb != NULL)
      n += sprintf(buffer+n, "orb%d/", current_orb->id);
    sprintf(buffer+n, "> ");

    line = orbs_shell_getline(buffer);

    // check if read EOF
    if (!line) {
      break;
    }

    args = orbs_shell_split_args(line);
    status = orbs_shell_execute(args);

    free(line);
    free(args);
  } while (status);

  // clear screen
  printf("\033[H\033[J");
}
