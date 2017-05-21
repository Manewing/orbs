#include "orb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

orb_t* create_orb(void) {
  orb_t* orb = (orb_t*)malloc(sizeof(orb_t));
  return reset_orb(orb);
}

orb_t* reset_orb(orb_t* orb) {
  memset(orb, 0, sizeof(orb_t));
  orb->x = rand() % W;
  orb->y = rand() % H;
  orb->score = 2000;
  orb->o = 'o';
  int _idx = 0;
  for (; _idx < ORB_GEN_SIZE; _idx++)
    orb->genes[_idx] = rand() % 256;
  return orb;
}
static int counter = 0;
void orb_live(orb_t* orb, map_t* map) {
  counter++;
  char instr = orb->genes[orb->idx];
  int r1 = (instr >> 6) & 0x3;
  int r2 = (instr >> 4) & 0x3;
  int jmp = 0;

  //    3
  //  2 O 0
  //    1
  static const int dirs[][2] = { {1,0}, {0,1}, {-1,0}, {0,-1} };
  static const char types[] = { 'o', 'O', '+', '#' };

  switch (instr & 0xF) {
    default:
      // sleep
      break;
    case 0x0:
      {

        // act
        // dd0x 0000
        char _d;

        if (instr & 0x10) {
          _d = r1 & 0x3;
        } else {
          _d = orb->regs[r1] & 0x3;
        }

        orb->x = wrap(orb->x + dirs[_d][0], W, 0);
        orb->y = wrap(orb->y + dirs[_d][1], H, 0);

      } break;
    case 0x1:
      {

        // sense
        // ttxx 0001
        int _x = wrap(orb->x + dirs[r2][0], W, 0);
        int _y = wrap(orb->y + dirs[r2][1], H, 0);
        char _t = types[r1];

        if (map->data[pos(_x, _y)] == _t)
          orb->status |= ORB_ZF | (1 << r2);

      } break;
    case 0x2:
      {

        // jmp
        // cc0x 0010
        // yyyy yyyy
        //
        if (r1 == 0x0 || r1 == ((orb->status >> 4) & 0x3)) {

          int _of = orb->genes[orb->idx+1];

          if (instr & 0x10)
            _of = -_of;

          orb->idx = (orb->idx + _of) & ORB_GEN_MASK;
          jmp = 1;
        }

        if (r1 != 0x0)
          orb->status &= ~0xf;

      } break;
    case 0x3:
      {

        // mov
        // r1r2 0011
        // vvvv vvvv (C)

        if (r1 == r2) {
          // immediate
          orb->idx = (orb->idx + 1) & ORB_GEN_MASK;
          orb->regs[r1] = orb->genes[orb->idx];
        } else {
          orb->regs[r1] = orb->regs[r2];
        }

      } break;
    case 0x4:
      {

        // add
        // r1r2 0100

        orb->regs[r1] += orb->regs[r2];
        if (orb->regs[r1] == 0)
          orb->status |= ORB_ZF;
        else if(orb->regs[r1] < 0)
          orb->status |= ORB_NF;

      } break;
    case 0x5:
      {

        // sub
        // r1r2 0101

        orb->regs[r1] -= orb->regs[r2];
        if (orb->regs[r1] == 0)
          orb->status |= ORB_ZF;
        else if(orb->regs[r1] < 0)
          orb->status |= ORB_NF;

      } break;
    case 0x6:
      {
        // inc/dec
        // r10x 0110

        if (instr & 0x10)
          orb->regs[r1]++;
        else
          orb->regs[r1]--;

        if (orb->regs[r1] == 0)
          orb->status |= ORB_ZF;
        else if(orb->regs[r1] < 0)
          orb->status |= ORB_NF;

      } break;

  }

  if (!jmp)
    orb->idx = (orb->idx + 1) & ORB_GEN_MASK;

  if (orb->score-- <= 0) {
    orb_die(orb, map);
    return;
  }

  map_update_orb(map, orb);

  //sprintf(map->data + pos(0, 23), "x: %d, y: %d, s: %d", orb->x, orb->y, orb->score);
}

void orb_feed(orb_t* orb, char food) {
  if (food == '+')
    orb->score += 1000;
  else if (food == '#')
    orb->score += 4000;

  orb->o = orb->score < 5000 ? 'o' : 'O';
}

void orb_die(orb_t* orb, map_t* map) {
  map_remove_orb(map, orb);
  free_orb(orb);
}

void orb_mutate(orb_t* orb) {
  int l = 0;
  for (; l < ORB_GEN_SIZE; l++) {
    if (rand() % 10000 == 1)
      orb->genes[l] = rand() % 256;
  }
}

orb_t* orb_crossover(orb_t* orb1, orb_t* orb2) {
  orb_t* new_orb = create_orb();
  int cut = rand() % ORB_GEN_SIZE;
  memcpy(new_orb->genes, orb1->genes, cut);
  memcpy(new_orb->genes + cut, orb2->genes + cut, ORB_GEN_SIZE - cut - 1);
  return new_orb;
}

void free_orb(orb_t* orb) {
  free(orb);
}
