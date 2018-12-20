#include "config_reader.h"
#include "defines.h"

#include <stdlib.h>
#include <string.h>

#define log_config_warn(fmt, ...) log_warn("CONFIG", fmt, ##__VA_ARGS__)
#define log_config_error(fmt, ...) log_error("CONFIG", fmt, ##__VA_ARGS__)

config_elem_t const *get_config_elem(config_reader_t const *cfg,
                                     const char *name) {
  for (unsigned int l = 0; l < cfg->elem_count; l++) {
    if (strcmp(cfg->elems[l].name, name) == 0) {
      return &cfg->elems[l];
    }
  }
  return NULL;
}

static void replace_char(char *str, char find, char replace) {
  while (*str) {
    if (*str == find) {
      *str = replace;
    }
    str++;
  }
}

int read_config_value(config_reader_t const *cfg, const char *name,
                      const char *value) {

  // get config element with given name
  config_elem_t const *elem = get_config_elem(cfg, name);
  if (elem == NULL) {
    log_config_error("unkown config parameter: '%s'\n", name);
    return -1;
  }

  // read config element
  if (sscanf(value, elem->type, elem->ptr) != 1) {
    log_config_error("cannot read parameter value (%s) for '%s'\n", value,
                     name);
    return -1;
  }

  return 0;
}

int read_config_line(config_reader_t const *cfg, const char *line) {
  int n;
  char buffer[256], linedup[512];

  // duplicate line remove unused characters from it
  strncpy(linedup, line, 512);
  linedup[511] = 0;
  replace_char(linedup, '=', ' ');
  replace_char(linedup, ',', ' ');

  // read config element name
  if (sscanf(linedup, "%256s%n", buffer, &n) != 1) {
    log_config_error("cannot read parameter from line: %s\n", line);
    return -1;
  }

  return read_config_value(cfg, buffer, linedup + n);
}

int read_config_file(config_reader_t const *cfg, const char *file) {
  char *line = NULL;
  size_t linecap, linecount = 0;
  int ret = 0;

  // open config file for reading
  FILE *fd = fopen(file, "r");
  if (fd == NULL) {
    log_config_error("could not open file %s for read\n", file);
    return -1;
  }

  // read all lines in config
  while (getline(&line, &linecap, fd) > 0) {
    linecount++;

    // skip empty lines and comments
    if (strcmp(line, "\n") == 0 || line[0] == '#')
      continue;

    // read line into config
    ret = read_config_line(cfg, line);

    // abort reading of file if we encoutered an error
    if (ret != 0) {
      log_config_error("syntax error in line %ld of file %s\n", linecount,
                       file);
      break;
    }
  }

  free(line);
  fclose(fd);
  return ret;
}

void print_config_options(config_reader_t const *cfg) {
  printf("Configuration information:\n");
  printf("%-30s %s\n", "# name", "# type    # doc");

  // print all configuration elements
  for (unsigned int l = 0; l < cfg->elem_count; l++) {
    printf("  %-30s %s        %s\n", cfg->elems[l].name, cfg->elems[l].type,
           cfg->elems[l].doc);
  }

  printf("\nTypes correspond to format parameters of 'scanf'.\n");
}
