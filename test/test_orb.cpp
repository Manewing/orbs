#include "test.hpp"

TEST(Orb, Create) {
  orb_t *orb = create_orb();

  for (int l = 0; l < 4; l++) {
    EXPECT_TRUE(orb->regs[l] == 0);
  }

  EXPECT_TRUE(orb->lr == 0);
  EXPECT_TRUE(orb->idx == 0);
  EXPECT_TRUE(orb->status == 0);

  free_orb(orb);
}
