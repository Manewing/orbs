#ifndef TEST_HPP
#define TEST_HPP

#include <gtest/gtest.h>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <orb.h>
#include <orb_instr.h>
}

namespace {

inline void dump_regs(orb_t const *orb) {
  printf("orb%d @ %p -> Register Dump:\n", orb->id, orb);
  for (int l = 0; l < 4; l++) {
    printf("  REG[%d] = 0x%x\n", l, orb->regs[l] & 0xff);
  }
  printf("  LR     = 0x%x\n", orb->lr & 0xff);
  printf("  STATUS = 0x%x\n", orb->status & 0xff);
}

inline void dump_disas(orb_t const *orb, int start, int end) {
  char buffer[64];

  printf("orb%d @ %p -> Instruction Dump [%d,%d]:\n", orb->id, orb, start, end);
  for (int l = start; l < end;) {
    l += orb_disas(orb, l, buffer);
    printf("  [0x%x]: %s\n", l, buffer);
  }
}

inline void orb_live_range(map_t *map, orb_t *orb, int idxs, int idxe) {
  orb->idx = idxs;
  while (orb->idx < idxe) {
    orb_live(orb, map);
  }
}

}

#endif // #ifndef TEST_HPP
