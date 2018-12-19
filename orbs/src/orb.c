#include "orb.h"
#include "orb_instr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "mm.h"

#define rand_char (rand() & 0xff)
#define rand_mutation (rand() % global_config.orb_mutation == 1)

static struct mm *orb_mm = NULL;

static int orb_id = 0;

orb_t *create_orb(void) {

  // check if orb memory manager is initialized
  if (orb_mm == NULL)
    orb_mm = create_mm(sizeof(orb_t));

  // create new orb and reset it
  orb_t *orb = (orb_t *)mm_elem_create(orb_mm);

  // set orb id
  orb->id = orb_id++;

  // reset orb
  reset_orb_genes(orb);

  return reset_orb(orb);
}

orb_t *reset_orb(orb_t *orb) {

  // reset orb state
  orb->idx = 0;
  orb->lr = 0;
  orb->status = 0;
  orb->regs[0] = 0;
  orb->regs[1] = 0;
  orb->regs[2] = 0;
  orb->regs[3] = 0;

  // set random position
  orb->x = rand() % W;
  orb->y = rand() % H;

  // initialize score
  orb->ttl = global_config.orb_ttl;
  orb->lifetime = 0;
  orb->score = global_config.orb_score;
  orb->body = global_config.orb_bodies[0];

  // reset trace
  orb->trace_count = 0;
  memset(orb->trace, 0, sizeof(orb->trace));

  return orb;
}

orb_t *reset_orb_genes(orb_t *orb) {
  int l;

  // create random genes
  for (l = 0; l < ORB_GENE_SIZE; l++)
    orb->genes[l] = rand_char;

  return orb;
}

void orb_live(orb_t *orb, map_t *map) {
  char instr = orb->genes[orb->idx];
  int r1 = (instr >> 6) & 0x3;
  int r2 = (instr >> 4) & 0x3;
  int jmp = 0;

  orb->trace[orb->idx]++;
  orb->trace_count++;

  //    3
  //  2 O 0
  //    1
  static const int dirs[][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
  static const char types[] = {'o', 'O', '+', '#'};

  switch (instr & 0xF) {
  default:
  case NOP_CODE:
    // nop
    break;
  case ACT_CODE: {

    // act
    // dd0x 0000
    int dir;

    if (instr & 0x10) {
      dir = r1;
    } else {
      dir = orb->regs[r1] & 0x3;
    }

    orb->x = wrap(orb->x + dirs[dir][0], W, 0);
    orb->y = wrap(orb->y + dirs[dir][1], H, 0);

    // acting costs more
    orb->score--;

  } break;
  case ACTCND_CODE: {

    // act
    // ccdd 0001

    if (r1 == ((orb->status >> 4) & 0x3)) {

      orb->x = wrap(orb->x + dirs[r2][0], W, 0);
      orb->y = wrap(orb->y + dirs[r2][1], H, 0);
    }

    CLEAR_ST(orb->status);

    // acting costs more
    orb->score--;

  } break;
  case 0xC:
  case SENSE_CODE: {

    // sense
    // ttxx 0010
    int _x = wrap(orb->x + dirs[r2][0], W, 0);
    int _y = wrap(orb->y + dirs[r2][1], H, 0);
    char _t = types[r1];

    orb->status &= 0xf0;
    orb->status &= ~ORB_ZF;

    if (_t == '+' || _t == '#') {
      if (map->data[pos(_x, _y)] == _t) {
        orb->status |= (1 << r2) | ORB_ZF;
      }
    } else {
      if (map_buffer(map, 1)[pos(_x, _y)] == _t)
        orb->status |= (1 << r2) | ORB_ZF;
    }

  } break;
  case 0xB: {
    // sense food
    // r1r2 1011
    orb->status &= ~ORB_ZF;

    int l = 0;
    int dist = (r2 == 0 || r2 == 2) ? W : H;

    int _x = orb->x;
    int _y = orb->y;
    for (; l < dist; l++) {
      _x = wrap(_x + dirs[r2][0], W, 0);
      _y = wrap(_y + dirs[r2][1], H, 0);

      char _at = map->data[pos(_x, _y)];
      if (_at != ' ') {
        orb->status |= ORB_ZF;
        orb->regs[r1] = (_at == '+') ? 0 : 1;
        break;
      }
    }

  } break;
  case JMP_CODE: {

    // jmp
    // cclx 0011
    // yyyy yyyy
    //

    // skip immediate
    orb->idx = NEXT_IDX(orb->idx);

    int cnd = 1;
    if (r1 != 0x0) {
      cnd = (r1 == ((orb->status >> 4) & 0x3));
      cnd = (instr & 0x10) ? !cnd : cnd;
    }

    if (cnd) {

      int _of = orb->genes[orb->idx];

      if (instr & 0x20) {
        orb->lr = NEXT_IDX(orb->idx);
      }

      orb->idx = (orb->idx + _of) & ORB_GENE_MASK;
      jmp = 1;
    }

    if (r1 != 0x0)
      orb->status &= ~(0x3 << 4);

  } break;
  case MOV_CODE: {

    // mov
    // r1r2 0100
    // vvvv vvvv (C)

    if (r1 == r2) {
      // immediate
      orb->idx = NEXT_IDX(orb->idx);
      orb->regs[r1] = orb->genes[orb->idx];
    } else {
      orb->regs[r1] = orb->regs[r2];
    }

  } break;
  case ADD_CODE: {

    // add
    // r1r2 0101

    orb->regs[r1] += orb->regs[r2];
    SET_ST_ARIT(orb->status, orb->regs[r1]);

  } break;
  case SUB_CODE: {

    // sub
    // r1r2 0110

    orb->regs[r1] -= orb->regs[r2];
    SET_ST_ARIT(orb->status, orb->regs[r1]);

  } break;
  case IDC_CODE: {
    // inc/dec
    // r10x 0111

    if (IDC_GET_X(instr)) {
      orb->regs[r1]++;
    } else {
      orb->regs[r1]--;
    }
    SET_ST_ARIT(orb->status, orb->regs[r1]);

  } break;
  case LDS_CODE: {
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
      orb->lr = orb->regs[r2];
      break;
    }

  } break;
  case 0xD:
  case 0x8: {
    // stat
    // xxxx 1000

    orb->status &= ~ORB_ZF;
    if ((instr >> 4) == (orb->status & 0xf))
      orb->status |= ORB_ZF;

  } break;
  case RET_CODE: {
    // return
    // ccxx 1010

    if (r1 == 0x0 || r1 == ((orb->status >> 4) & 0x3)) {
      orb->idx = orb->lr;
      jmp = 1;
    }

  } break;
  }

  if (!jmp) {
    orb->idx = NEXT_IDX(orb->idx);
  }

  if (orb->score-- <= 0 || ++orb->lifetime >= orb->ttl) {
    orb_die(orb, map);
    return;
  }

  map_update_orb(map, orb);
}

int orb_disas(orb_t const *orb, int idx, char buffer[64]) {
  uint8_t instr = orb->genes[idx];
  int r1 = (instr >> 6) & 0x3;
  int r2 = (instr >> 4) & 0x3;

  static const char *dirs[] = {"right", "down", "left", "up"};
  static const char *types[] = {"'o'", "'O'", "'+'", "'#'"};

  switch (instr & 0xF) {
  default:
  case NOP_CODE:
    sprintf(buffer, "nop");
    return 1;
  case ACT_CODE: {

    // act
    // dd0x 0000
    if (instr & 0x10)
      sprintf(buffer, "act\tdir[r%d]", r1);
    else
      sprintf(buffer, "act\t%s", dirs[r1]);
  }
    return 1;
  case ACTCND_CODE: {

    // act
    // ccdd 0001
    int _n = sprintf(buffer, "act\t%s", dirs[r2]);

    if (r1 != 0x0)
      sprintf(buffer + _n, "\t\tif status & 0x%x0", r1);
  }
    return 1;
  case SENSE_CODE: {

    // sense
    // ttxx 0010
    sprintf(buffer, "sense\t%s,\t%s", dirs[r2], types[r1]);
  }
    return 1;
  case 0xB: {
    // sense food
    // r1r2 1011

    sprintf(buffer, "sense\t%s,\tr%d", dirs[r2], r1);
  }
    return 1;
  case JMP_CODE: {

    // jmp
    // cclx 0011
    // yyyy yyyy

    int _of = orb->genes[(orb->idx + 1) & ORB_GENE_MASK];
    int _idx = (idx + _of) & ORB_GENE_MASK;

    char _l = (instr & 0x20) ? 'l' : ' ';
    const char *_c = (instr & 0x10) ? "!" : "";

    int _n = sprintf(buffer, "jmp%c\t0x%02x", _l, _idx);

    if (r1 != 0x0) {
      sprintf(buffer + _n, "\t\tif %sstatus & 0x%x0", _c, r1);
    }
  }
    return 2;
  case MOV_CODE: {

    // mov
    // r1r2 0100
    // vvvv vvvv (C)

    if (r1 == r2) {
      int _imm = orb->genes[(orb->idx + 1) & ORB_GENE_MASK];
      sprintf(buffer, "mov\tr%d,\t0x%02x", r1, _imm);
      return 2;
    } else {
      sprintf(buffer, "mov\tr%d,\tr%d", r1, r2);
      return 1;
    }
  }
  case ADD_CODE: {

    // add
    // r1r2 0101

    sprintf(buffer, "add\tr%d,\tr%d", r1, r2);
  }
    return 1;
  case SUB_CODE: {

    // sub
    // r1r2 0110

    sprintf(buffer, "sub\tr%d,\tr%d", r1, r2);
  }
    return 1;
  case IDC_CODE: {
    // inc/dec
    // r10x 0111

    sprintf(buffer, "%s\tr%d", (instr & 0x10) ? "inc" : "dec", r1);
  }
    return 1;
  case LDS_CODE: {
    // load/store
    // r1r2 1001

    switch (r1) {
    case 0x0:
      sprintf(buffer, "load\tr%d,\tstatus", r2);
      break;
    case 0x1:
      sprintf(buffer, "load\tr%d,\tscore", r2);
      break;
    case 0x2:
      sprintf(buffer, "store\tidx,\tr%d", r2);
      break;
    case 0x3:
      sprintf(buffer, "store\tlr,\tr%d", r2);
      break;
    }
  }
    return 1;
  case RET_CODE: {
    // return
    // cc00 1010

    int _n = sprintf(buffer, "ret\t\t");

    if (r1 != 0x0)
      sprintf(buffer + _n, "\tif status & 0x%x0", r1);
  }
    return 1;
  }

  return 1;
}

void orb_feed(orb_t *orb, char food) {

  // update score depending on food type
  if (food == global_config.food_types[0])
    orb->score += global_config.food_scores[0];
  else if (food == global_config.food_types[1])
    orb->score += global_config.food_scores[1];

  // update orb body depending on score
  if (!orb->highlight) {
    if (orb->score >= global_config.orb_scores[2])
      orb->body = global_config.orb_bodies[2];
    else if (orb->score >= global_config.orb_scores[1])
      orb->body = global_config.orb_bodies[1];
    else
      orb->body = global_config.orb_bodies[0];
  }
}

void orb_die(orb_t *orb, map_t *map) {

  // remove orb from map
  map_remove_orb(map, orb);

  // free orb
  free_orb(orb);
}

void orb_mutate(orb_t *orb) {
  int l = 0;

  // randomly mutate some of the genes
  for (; l < ORB_GENE_SIZE; l++) {
    if (rand_mutation)
      orb->genes[l] ^= rand_char;
  }
}

orb_t *orb_crossover(orb_t *orb1, orb_t *orb2) {
  orb_t *new_orb = create_orb();
  int cut = rand() & ORB_GENE_MASK;

  // crossover genes from parents
  memcpy(new_orb->genes, orb1->genes, cut);
  memcpy(new_orb->genes + cut, orb2->genes + cut, ORB_GENE_SIZE - cut - 1);

  return new_orb;
}

float get_orb_instr_usage(orb_t const *orb) {
  int idx = 0, count = 0;

  while (idx < ORB_GENE_SIZE) {
    count += ((float)orb->trace[idx] / orb->trace_count < .0001 ? 0 : 1);
    idx++;
  }

  return (100.0 * count) / ORB_GENE_SIZE;
}

void free_orb(void *orb) { mm_elem_free(orb_mm, orb); }
