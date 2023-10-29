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

TEST(MySuite, pass) {
  ASSERT_EQ(1, 1);
}

TEST(MySuite, pass_fail_pass) {
  ASSERT_EQ(1, 1);
  ASSERT_EQ(2, 1);
  ASSERT_EQ(1, 1);
}

TEST(MySuite, pass_fail) {
  ASSERT_EQ(1, 1);
  ASSERT_EQ(2, 1);
}

TEST(MySuite, pass_crash) {
  ASSERT_EQ(1, 1);
  raise(SIGSEGV);
}

TEST(EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE) {
  ASSERT_EQ(1, 1);
}

TEST(arr, arr1) {
  int a[3] = {1,2,3};
  int b[3] = {1,2,3};
  ASSERT_ARR_EQ(a, b, 3);
}
