#include "shell.h"

#include "map.h"
#include "orb.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

extern map_t* map;

static char* orbs_shell_getline(char* pref) {
  char* line = readline(pref);
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
  printf("  [Score]  : %d\n", orb->score);
  printf("  [TTL]    : %d\n", orb->ttl);
  printf("  [Status] : 0x%02x\n", orb->status);
  printf("  [Regs]   : r0: 0x%02x, r1: 0x%02x, r2: 0x%02x, r3: 0x%02x\n",
      orb->regs[0], orb->regs[1], orb->regs[2], orb->regs[3]);
  printf("  [LR]     : 0x%02x\n", orb->lr);
  printf("  [Idx]    : 0x%02x\n", orb->idx);
}

static orb_t * current_orb = NULL;

static int print_orb_disas_instr(orb_t* orb, int idx) {
  int n;
  char buffer[64];

  // disassemble instruction
  n = orb_disas(orb, idx, buffer);

  if (strcmp(buffer, "nop") == 0)
    printf("\e[37m");

  printf("[0x%02x]: 0x%02x %s\n\e[39m", idx, orb->genes[idx], buffer);

  return n;
}

static int orbs_cmd_disas(char** args) {

  if (current_orb != NULL) {
    printf("%p -> genes @ map/%s/\n", current_orb, args[1]);

    int idx = 0;
    while (idx < ORB_GENE_SIZE) {
      if (idx == current_orb->idx)
        printf(" -> ");
      else
        printf("    ");
      idx += print_orb_disas_instr(current_orb, idx);
    }

    return 1;
  }

  printf("no orb selected\n");
  return 1;
}

static int orbs_cmd_p(char** args) {
  if (current_orb != NULL) {
    print_orb_status(current_orb);
    return 1;
  }

  printf("no orb selected\n");
  return 1;
}

static int orbs_cmd_s(char** args) {
  if (current_orb != NULL) {
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
  printf("no orb selected\n");
  return 1;
}

static int orbs_cmd_ls_orb(char* arg) {

  // get orb
  orb_t* orb = str2orb(arg);

  if (orb != NULL) {
    printf("%8p  ->  map/%s/ @ [%2d, %2d]\n", orb, arg, orb->x, orb->y);
    print_orb_status(orb);
    return 1;
  }

  printf("no such orb: %s\n", arg);
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

static int orbs_cmd_ls(char** args) {
  if (args[1] == NULL && current_orb == NULL)
    return orbs_cmd_ls_map();
  else if (args[1] != NULL)
    return orbs_cmd_ls_orb(args[1]);
  else {
    int l, idx = current_orb->idx - 4;
    for (l = 0; l < 9; l++) {
      int _idx = (idx+l) & 0xff;

      if (_idx == current_orb->idx)
        printf(" -> ");
      else
        printf("    ");

      print_orb_disas_instr(current_orb, _idx);
    }
    return 1;
  }
}

static int orbs_cmd_cd(char** args) {
  if (args[1] == NULL)
    current_orb = NULL;
  else if (strcmp(args[1], "..") == 0) {
    current_orb = NULL;
  } else {
    orb_t* orb = str2orb(args[1]);

    if (orb == NULL)
      printf("no such orb: %s\n", args[1]);
    else
      current_orb = orb;
  }

  return 1;
}

static int orbs_cmd_c(char** args) {
  return 0;
}

static const char* orbs_command_strs[] = {
  "ls",
  "l",
  "cd",
  "p",
  "s",
  "disas",
  "c"
};

static int (*orbs_command_funcs[])(char**) = {
  &orbs_cmd_ls,
  &orbs_cmd_ls,
  &orbs_cmd_cd,
  &orbs_cmd_p,
  &orbs_cmd_s,
  &orbs_cmd_disas,
  &orbs_cmd_c
};

static int orbs_commands(void) {
  return sizeof(orbs_command_strs) / sizeof(char*);
}

static int orbs_shell_execute(char** args) {
  if (args[0] == NULL)
    return 1;

  int l;
  for (l = 0; l < orbs_commands(); l++) {
    if (strcmp(args[0], orbs_command_strs[l]) == 0)
      return orbs_command_funcs[l](args);
  }

  printf("unkown command: %s\n", args[0]);

  return 1;
}

void orbs_shell(void) {
  int status;
  char* line;
  char** args;

  char buffer[256];

  do {
    int n = sprintf(buffer, "orbs/map/");
    if (current_orb != NULL)
      n += sprintf(buffer+n, "orb%d/", current_orb->id);
    sprintf(buffer+n, "> ");

    line = orbs_shell_getline(buffer);
    args = orbs_shell_split_args(line);
    status = orbs_shell_execute(args);

    free(line);
    free(args);
  } while (status);
}
