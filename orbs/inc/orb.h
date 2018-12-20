#ifndef ORB_HH
#define ORB_HH

#include "map.h"

#include <stdint.h>

// status flags
#define ORB_ZF 0x10
#define ORB_NF 0x20

#define ORB_GENE_SIZE 256
#define ORB_GENE_MASK 0xFF

typedef struct orb_t {
  int x;
  int y;

  char body;
  int score;
  int lifetime;
  int ttl;

  uint8_t status;
  int8_t regs[4];
  int lr;
  uint8_t idx;
  uint8_t genes[ORB_GENE_SIZE];

  int trace[ORB_GENE_SIZE];
  int trace_count;

  int id;
  int highlight;
} orb_t;

orb_t *create_orb(void);
orb_t *reset_orb(orb_t *orb);
orb_t *reset_orb_genes(orb_t *orb);

void orb_live(orb_t *orb, map_t *map);
int orb_disas(orb_t const *orb, int idx, char buffer[64]);
void orb_feed(orb_t *orb, char food);
void orb_die(orb_t *orb, map_t *map);

void orb_mutate(orb_t *orb);
orb_t *orb_crossover(orb_t *orb1, orb_t *orb2);

float get_orb_instr_usage(orb_t const *orb);

void free_orb(void *orb);

#endif // #ifndef ORB_HH
