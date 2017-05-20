#ifndef ORB_HH
#define ORB_HH

#include "map.h"

#define ORB_BODY        'O'
#define ORB_GEN_SIZE    256
#define ORB_GEN_MASK   0xFF

#define ORB_ZF 0x10
#define ORB_NF 0x20

typedef struct orb_t {
  int x;
  int y;

  char o;
  int score;
  int idx;

  // 0-3: nb
  //   4: ZF
  //   5: NF
  char status;
  char regs[4];
  char genes[ORB_GEN_SIZE];
} orb_t;

orb_t* create_orb(void);
orb_t* reset_orb(orb_t* orb);

void orb_live(orb_t* orb, map_t* map);
void orb_die(orb_t* orb, map_t* map);

void free_orb(orb_t* orb);

#endif // #ifndef ORB_HH
