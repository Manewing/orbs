#include "test.hpp"

extern "C" {
#include "list.h"
#include "mm.h"
int free_idx;
void *free_buffer[4];
void free_to_buffer(void *data) { free_buffer[free_idx++] = data; }
void free_dummy(void *) {}
};

TEST(List, Add) {
  list_t list = LIST_INIT;
  EXPECT_EQ(list.head, nullptr);
  EXPECT_EQ(list.size, 0);

  list_add(&list, (void *)0x1);
  list_add(&list, (void *)0x2);
  list_add(&list, (void *)0x3);
  list_add(&list, (void *)0x4);

  EXPECT_EQ(list.size, 4);
  EXPECT_EQ(list.head->next->next->next->next, nullptr);

  node_t *node = list.head;
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->data, (void *)0x4);

  node = node->next;
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->data, (void *)0x3);

  node = node->next;
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->data, (void *)0x2);

  node = node->next;
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->data, (void *)0x1);

  list_print(&list);
  list_remove_all(&list, free_dummy);
}

TEST(List, Remove) {
  list_t list = LIST_INIT;
  list_add(&list, (void *)0x1);
  list_add(&list, (void *)0x2);
  list_add(&list, (void *)0x3);
  list_add(&list, (void *)0x4);
  EXPECT_EQ(list.size, 4);

  EXPECT_EQ(list_remove(&list, (void *)0x5), 0);
  EXPECT_EQ(list.size, 4);

  EXPECT_EQ(list_remove(&list, (void *)0x3), 1);
  EXPECT_EQ(list.size, 3);

  node_t *node = list.head;
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->data, (void *)0x4);

  node = node->next;
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->data, (void *)0x2);

  node = node->next;
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(node->data, (void *)0x1);

  EXPECT_EQ(node->next, nullptr);
  list_remove_all(&list, free_dummy);
}

TEST(List, RemoveAll) {
  list_t list = LIST_INIT;
  list_add(&list, (void *)0x1);
  list_add(&list, (void *)0x2);
  list_add(&list, (void *)0x3);
  list_add(&list, (void *)0x4);

  list_remove_all(&list, free_to_buffer);
  EXPECT_EQ(list.size, 0);
  EXPECT_EQ(list.head, nullptr);
  EXPECT_EQ(free_idx, 4);
  EXPECT_EQ(free_buffer[0], (void *)0x4);
  EXPECT_EQ(free_buffer[1], (void *)0x3);
  EXPECT_EQ(free_buffer[2], (void *)0x2);
  EXPECT_EQ(free_buffer[3], (void *)0x1);
}
