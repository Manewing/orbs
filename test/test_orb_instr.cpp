#include "test.hpp"

// Calculates sum of natural numbers until count and checks the
// result.
TEST(Orb, InstructionAddJmpSubMovi) {
  map_t *map = create_map();
  orb_t *orb = create_orb();

  // set count
  const int COUNT = 0xf;

  // clear genes
  memset(orb->genes, NOP, ORB_GENE_SIZE);

  // 0x00:  mov r3, $COUNT
  // 0x01:  COUNT
  // 0x02:  inc r0
  // 0x03:  add r1, r0
  // 0x04:  mov r2, r0
  // 0x05:  sub r2, r3
  // 0x06:  jmp 0x2      if ! status & ORB_ZF
  // 0x07:  JMP_IMD

  int idx = 0;
  orb->genes[idx++] = MOVI(REG3);
  orb->genes[idx++] = IMD(COUNT);
  orb->genes[idx++] = INC(REG0);
  orb->genes[idx++] = ADD(REG1, REG0);
  orb->genes[idx++] = MOV(REG2, REG0);
  orb->genes[idx++] = SUB(REG2, REG3);
  orb->genes[idx++] = JMP_IFN(ZF_BIT);
  orb->genes[idx] = JMP_ADDR(idx, 0x2);
  idx++;

  dump_disas(orb, 0, idx);
  orb_live_range(map, orb, 0, idx);
  dump_regs(orb);

  EXPECT_TRUE((orb->regs[REG0] & 0xff) == COUNT);
  EXPECT_TRUE((orb->regs[REG1] & 0xff) == ((COUNT + 1) * COUNT) / 2);
  EXPECT_TRUE((orb->regs[REG2] & 0xff) == 0x00);
  EXPECT_TRUE((orb->regs[REG3] & 0xff) == COUNT);
  EXPECT_TRUE((orb->lr & 0xff) == 0x0);
  EXPECT_TRUE((orb->status & 0xff) == 0x0);

  free_orb(orb);
  free_map(map);
}
