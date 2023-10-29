#include "souffle.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

TEST(MySuite, TestCase1) { ASSERT_TRUE(false); }

TEST(MySuite, test_number_eq) {
  int a = 5;
  int b = 1;
  ASSERT_EQ(a, b);
}
TEST(MySuite, fffff) {
  // ASSERT_EQ(12, 1);
  raise(SIGSEGV);
}

TEST(MySuite, pass_me) {
  ASSERT_EQ(1, 1);
  ASSERT_EQ(2, 1);
}

TEST(MySuite, 1) {
  ASSERT_EQ(1, 1);
  ASSERT_EQ(2, 1);
}
TEST(MySuite, 2) {
  ASSERT_EQ(1, 1);
  ASSERT_EQ(2, 1);
}
TEST(MySuite, 3) {
  ASSERT_EQ(1, 1);
}
TEST(EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE) {
  ASSERT_EQ(1, 1);
}

