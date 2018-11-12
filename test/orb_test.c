#include "orb.h"
#include "orb_instr.h"
#include "string.h"
#include "test.h"

map_t *map;

void dump_regs(orb_t const *orb) {
  printf("orb%d @ %p -> Register Dump:\n", orb->id, orb);
  for (int l = 0; l < 4; l++) {
    printf("  REG[%d] = 0x%x\n", l, orb->regs[l] & 0xff);
  }
  printf("  LR     = 0x%x\n", orb->lr & 0xff);
  printf("  STATUS = 0x%x\n", orb->status & 0xff);
}

void dump_disas(orb_t const *orb, int start, int end) {
  char buffer[64];

  printf("orb%d @ %p -> Instruction Dump [%d,%d]:\n", orb->id, orb, start, end);
  for (int l = start; l < end;) {
    l += orb_disas(orb, l, buffer);
    printf("  [0x%x]: %s\n", l, buffer);
  }
}

void orb_live_range(orb_t *orb, int idxs, int idxe) {
  orb->idx = idxs;
  while (orb->idx < idxe) {
    orb_live(orb, map);
  }
}

int orb_create_test() {
  int failed = 0;

  orb_t *orb = create_orb();

  for (int l = 0; l < 4; l++) {
    EXPECT_TRUE(orb->regs[l] == 0);
  }

  EXPECT_TRUE(orb->lr == 0);
  EXPECT_TRUE(orb->idx == 0);
  EXPECT_TRUE(orb->status == 0);

  free_orb(orb);
  return failed;
}

// Calculates sum of natural numbers until count and checks the
// result.
int orb_instr_test_0(const int COUNT) {
  int failed = 0;

  orb_t *orb = create_orb();

  // clear genes
  memset(orb->genes, NOP, ORB_GENE_SIZE);

  // mov r3, $COUNT
  // inc r0
  // add r1, r0
  // mov r2, r0
  // sub r2, r3
  // jmp 0x2      if ! status & ORB_ZF

  int idx = 0;
  orb->genes[idx++] = MOVI(REG3);
  orb->genes[idx++] = IMD(COUNT);
  orb->genes[idx++] = INC(REG0);
  orb->genes[idx++] = ADD(REG1, REG0);
  orb->genes[idx++] = MOV(REG2, REG0);
  orb->genes[idx++] = SUB(REG2, REG3);
  orb->genes[idx++] = JMP_IFN(ZF_BIT);
  orb->genes[idx++] = JMP_ADDR(idx, 0x2);

  dump_disas(orb, 0, idx);
  orb_live_range(orb, 0, idx);
  dump_regs(orb);

  EXPECT_TRUE((orb->regs[REG0] & 0xff) == COUNT);
  EXPECT_TRUE((orb->regs[REG1] & 0xff) == ((COUNT + 1) * COUNT) / 2);
  EXPECT_TRUE((orb->regs[REG2] & 0xff) == 0x00);
  EXPECT_TRUE((orb->regs[REG3] & 0xff) == COUNT);
  EXPECT_TRUE((orb->lr & 0xff) == 0x0);
  EXPECT_TRUE((orb->status & 0xff) == 0x0);

  free_orb(orb);
  return failed;
}

int main() {
  map = create_map();

  CHECK_TEST(orb_create_test(), 0);
  CHECK_TEST(orb_instr_test_0(0xf), 0);

  free_map(map);
  return 0;
}
