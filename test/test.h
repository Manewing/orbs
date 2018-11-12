#ifndef TEST_HH
#define TEST_HH

#include <stdio.h>

#define ASSERT_TRUE(stmt)                                                      \
  if (!(stmt)) {                                                               \
    printf("ASSERT_TRUE(" #stmt ") failed\n");                                 \
    return 1;                                                                  \
  }

#define ASSERT_FALSE(stmt)                                                     \
  if ((stmt)) {                                                                \
    printf("ASSERT_FALSE(" #stmt ") failed\n");                                \
    return 1;                                                                  \
  }

#define EXPECT_TRUE(stmt)                                                      \
  if (!(stmt)) {                                                               \
    printf("EXPECT_TRUE(" #stmt ") failed\n");                                 \
    failed = 1;                                                                \
  }

#define EXPECT_FALSE(stmt)                                                     \
  if ((stmt)) {                                                                \
    printf("EXPECT_FALSE(" #stmt ") failed\n");                                \
    failed = 1;                                                                \
  }

#define EXPECT_EQ_BASE(a, b, msg)                                              \
  if (!((a) == (b))) {                                                         \
    ::std::cerr << "ERROR" << msg << ": expected '" << #a << "' = '" << a      \
                << "' to be equal to '" << #b << "' = '" << b << "'"           \
                << ::std::endl;                                                \
    Success = false;                                                           \
  }
#define EXPECT_EQ(a, b) EXPECT_EQ_BASE(a, b, "")
#define EXPECT_EQ_MSG(a, b, msg) EXPECT_EQ_BASE(a, b, "[" << msg << "]")

#define CHECK_TEST(test, ret)                                                  \
  printf("TEST(" #test "): Run\n");                                            \
  if ((test) == (ret)) {                                                       \
    printf("TEST(" #test "): Success\n");                                      \
  } else {                                                                     \
    printf("TEST(" #test "): failed\n");                                       \
  }

#endif // #ifndef TEST_HH
