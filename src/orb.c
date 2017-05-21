#include "orb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "config.h"

#define rand_char       (rand() % 256)
#define rand_mutation   (rand() % global_config.orb_mutation == 1)

static struct mm* orb_mm = NULL;

orb_t* create_orb(void) {

  // check if orb memory manager is initialized
  if (orb_mm == NULL)
    orb_mm = create_mm(sizeof(orb_t));

  // create new orb and reset it
  orb_t* orb = (orb_t*)mm_elem_create(orb_mm);

  return reset_orb(orb);
}

orb_t* reset_orb(orb_t* orb) {

  // set random position
  orb->x      = rand() % W;
  orb->y      = rand() % H;

  // initialize score
  orb->score  = global_config.orb_score;
  orb->body   = global_config.orb_bodies[0];

  return orb;
}

orb_t* reset_orb_genes(orb_t* orb) {
  int l;

  // create random genes
  for (l = 0; l < ORB_GENE_SIZE; l++)
    orb->genes[l] = rand_char;

  return orb;
}

void orb_live(orb_t* orb, map_t* map) {
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

        // act
        // ccdd 0001

        if (r1 == 0x0 || r1 == ((orb->status >> 4) & 0x3)) {

          orb->x = wrap(orb->x + dirs[r2][0], W, 0);
          orb->y = wrap(orb->y + dirs[r2][1], H, 0);

        }

        if (r1 != 0x0)
          orb->status &= 0xf;

      } break;
    case 0x2:
      {

        // sense
        // ttxx 0010
        int _x = wrap(orb->x + dirs[r2][0], W, 0);
        int _y = wrap(orb->y + dirs[r2][1], H, 0);
        char _t = types[r1 & 0x3];

        if (map->data[pos(_x, _y)] == _t)
          orb->status |= (1 << r2);

      } break;
    case 0x3:
      {

        // jmp
        // cclx 0011
        // yyyy yyyy
        //
        if (r1 == 0x0 || r1 == ((orb->status >> 4) & 0x3)) {

          int _of = orb->genes[orb->idx+1];

          if (instr & 0x10)
            _of = -_of;
          if (instr & 0x20)
            orb->lr = (orb->idx + 1) & ORB_GENE_MASK;

          orb->idx = (orb->idx + _of) & ORB_GENE_MASK;
          jmp = 1;
        }

        if (r1 != 0x0)
          orb->status &= 0xf;

      } break;
    case 0x4:
      {

        // mov
        // r1r2 0100
        // vvvv vvvv (C)

        if (r1 == r2) {
          // immediate
          orb->idx = (orb->idx + 1) & ORB_GENE_MASK;
          orb->regs[r1] = orb->genes[orb->idx];
        } else {
          orb->regs[r1] = orb->regs[r2];
        }

      } break;
    case 0x5:
      {

        // add
        // r1r2 0101

        orb->regs[r1] += orb->regs[r2];
        if (orb->regs[r1] == 0)
          orb->status |= ORB_ZF;
        else if(orb->regs[r1] < 0)
          orb->status |= ORB_NF;

      } break;
    case 0x6:
      {

        // sub
        // r1r2 0110

        orb->regs[r1] -= orb->regs[r2];
        if (orb->regs[r1] == 0)
          orb->status |= ORB_ZF;
        else if(orb->regs[r1] < 0)
          orb->status |= ORB_NF;

      } break;
    case 0x7:
      {
        // inc/dec
        // r1lx 0111

        if (instr & 0x10)
          orb->regs[r1]++;
        else
          orb->regs[r1]--;

        if (orb->regs[r1] == 0)
          orb->status |= ORB_ZF;
        else if(orb->regs[r1] < 0)
          orb->status |= ORB_NF;

      } break;
    case 0x8:
      {
        // stat
        // xxxx 1000

        if ((instr >> 4) == (orb->status & 0xf))
            orb->status |= ORB_ZF;
        orb->status &= ~0xf;

      } break;
    case 0x9:
      {
        // load/store
        // r1r2 1001

        switch (r1) {
          case 0x0:
            orb->regs[r2] = orb->status;
            break;
          case 0x1:
            orb->regs[r2] = orb->score / 1000;
            break;
          case 0x2:
            orb->idx = orb->regs[r2];
            break;
          case 0x3:
            orb->regs[r2] = orb->regs[r1];
            break;
         }

      } break;
    case 0xA:
      {
        // return
        // 0000 1010

        orb->idx = orb->lr;
        jmp = 1;

      } break;
  }

  if (rand_mutation) {
    int idx = rand() & ORB_GENE_MASK;
    orb->genes[idx] = rand_char;
  }


  if (!jmp)
    orb->idx = (orb->idx + 1) & ORB_GENE_MASK;

  if (orb->score-- <= 0) {
    orb_die(orb, map);
    return;
  }

  map_update_orb(map, orb);
}

void orb_feed(orb_t* orb, char food) {

  // update score depending on food type
  if (food == global_config.food_types[0])
    orb->score += global_config.food_scores[0];
  else if (food == global_config.food_types[1])
    orb->score += global_config.food_scores[1];

  // update orb body depending on score
  if (orb->score >= global_config.orb_scores[2])
    orb->body = global_config.orb_bodies[2];
  else if (orb->score >= global_config.orb_scores[1])
    orb->body = global_config.orb_bodies[1];
  else
    orb->body = global_config.orb_bodies[0];

}

void orb_die(orb_t* orb, map_t* map) {

  // remove orb from map
  map_remove_orb(map, orb);

  // free orb
  free_orb(orb);

}

void orb_mutate(orb_t* orb) {
  int l = 0;

  // randomly mutate some of the genes
  for (; l < ORB_GENE_SIZE; l++) {
    if (rand_mutation)
      orb->genes[l] = rand_char;
  }

}

orb_t* orb_crossover(orb_t* orb1, orb_t* orb2) {
  orb_t* new_orb = create_orb();
  int cut = rand() & ORB_GENE_MASK;

  // crossover genes from parents
  memcpy(new_orb->genes, orb1->genes, cut);
  memcpy(new_orb->genes + cut, orb2->genes + cut, ORB_GENE_SIZE - cut - 1);

  return new_orb;
}

void free_orb(void* orb) {
  mm_elem_free(orb_mm, orb);
}
