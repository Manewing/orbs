#include "test.hpp"
#include <string>

extern "C" {
#include "config.h"
};

TEST(Config, GetConfigElem) {
  config_elem_t const *Elem = nullptr;
  auto *Reader = &global_config_reader;

  Elem = get_config_elem(Reader, "asdf");
  EXPECT_EQ(Elem, nullptr);

  Elem = get_config_elem(Reader, "global_config.skip");
  ASSERT_TRUE(Elem != nullptr);
  EXPECT_EQ(*reinterpret_cast<int *>(Elem->ptr), 0);

  Elem = get_config_elem(Reader, "global_config.herz");
  ASSERT_TRUE(Elem != nullptr);
  EXPECT_EQ(*reinterpret_cast<int *>(Elem->ptr), 201);

  Elem = get_config_elem(Reader, "orb_config.scores[1]");
  ASSERT_TRUE(Elem != nullptr);
  EXPECT_EQ(*reinterpret_cast<int *>(Elem->ptr), 4000);

  Elem = get_config_elem(Reader, "global_config.stats");
  ASSERT_TRUE(Elem != nullptr);
  EXPECT_EQ(reinterpret_cast<const char *>(Elem->ptr), ::std::string(""));
}

TEST(Config, ReadValue) {
  config_elem_t const *Elem = nullptr;
  auto *Reader = &global_config_reader;

  EXPECT_EQ(read_config_value(Reader, "asdf", "abc"), -1);
  EXPECT_EQ(read_config_value(Reader, "orb_config.score", "abc"), -1);

  Elem = get_config_elem(Reader, "orb_config.score");
  ASSERT_TRUE(Elem != nullptr);
  EXPECT_EQ(read_config_value(Reader, "orb_config.score", "42"), 0);
  EXPECT_EQ(*reinterpret_cast<int *>(Elem->ptr), 42);

  Elem = get_config_elem(Reader, "global_config.stats");
  ASSERT_TRUE(Elem != nullptr);
  EXPECT_EQ(read_config_value(Reader, "global_config.stats", "abc"), 0);
  EXPECT_EQ(reinterpret_cast<const char *>(Elem->ptr), ::std::string("abc"));
}

TEST(Config, ReadLine) {
  config_elem_t const *Elem = nullptr;
  auto *Reader = &global_config_reader;

  EXPECT_EQ(read_config_line(Reader, "orb_config.count"), -1);

  Elem = get_config_elem(Reader, "orb_config.count");
  ASSERT_TRUE(Elem != nullptr);
  EXPECT_EQ(read_config_line(Reader, "orb_config.count=32"), 0);
  EXPECT_EQ(*reinterpret_cast<int *>(Elem->ptr), 32);
}

TEST(Config, ConfigFiles) {
  auto *Reader = &global_config_reader;
  EXPECT_EQ(read_config_file(Reader, CSTR(ROOT_DIR + "/cfg/default.cfg")), 0);
  EXPECT_EQ(read_config_file(Reader, CSTR(ROOT_DIR + "/cfg/complex.cfg")), 0);
}
