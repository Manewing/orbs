#ifndef ORB_HH
#define ORB_HH

#include "map.h"

// status flags
#define ORB_RF 0x01
#define ORB_DF 0x02
#define ORB_LF 0x04
#define ORB_TF 0x08
#define ORB_ZF 0x10
#define ORB_NF 0x20

#define ORB_GENE_SIZE    256
#define ORB_GENE_MASK   0xFF

typedef struct orb_t {
  int     x;
  int     y;

  char    body;
  int     score;
  int     ttl;

  char    status;
  char    regs[4];
  int     lr;
  int     idx;
  char    genes[ORB_GENE_SIZE];
} orb_t;


orb_t*    create_orb(void);
orb_t*    reset_orb(orb_t* orb);
orb_t*    reset_orb_genes(orb_t* orb);

void      orb_live(orb_t* orb, map_t* map);
void      orb_feed(orb_t* orb, char food);
void      orb_die(orb_t* orb, map_t* map);

void      orb_mutate(orb_t* orb);
orb_t*    orb_crossover(orb_t* orb1, orb_t* orb2);

void      free_orb(void* orb);

#endif // #ifndef ORB_HH
