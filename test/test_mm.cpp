#include "test.hpp"

extern "C" {
#include "mm.h"
};

TEST(MM, Create) {
  mm *m = create_mm(sizeof(int));
  EXPECT_EQ(m->elem_size, sizeof(int));
  EXPECT_EQ(m->buffer_idx, 0);
  EXPECT_EQ(m->buffer_size, 256);
  ASSERT_TRUE(m->buffer != nullptr);
  free_mm(m);
}

TEST(MM, Elems) {
  void *elem;
  mm *m = create_mm(sizeof(int));

  EXPECT_EQ(m->buffer_idx, 0);
  elem = mm_elem_create(m);
  mm_elem_free(m, elem);
  EXPECT_EQ(m->buffer[0], elem);
  EXPECT_EQ(m->buffer_idx, 1);

  EXPECT_EQ(mm_elem_create(m), elem);
  EXPECT_EQ(m->buffer_idx, 0);
  mm_elem_free(m, elem);

  free_mm(m);
}

TEST(MM, ElemOverflow) {
  mm *m = create_mm(sizeof(int));

  // max free capacity is 256, 300 will trigger a realloc
  for (int l = 0; l < 300; l++) {
    mm_elem_free(m, reinterpret_cast<void *>(l));
  }
  for (int l = 0; l < 300; l++) {
    EXPECT_EQ(m->buffer[l], reinterpret_cast<void *>(l));
  }
  EXPECT_EQ(m->buffer_idx, 300);
  EXPECT_EQ(m->buffer_size, 512);

  m->buffer_idx = 0;
  free_mm(m);
}
