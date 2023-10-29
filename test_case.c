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
  ASSERT_EQ(12, 1);
  raise(SIGSEGV);
}
// TEST(MySuite, FUCK) {
//      int a = 1;
//      int b = 2;
//      ASSERT_LT(a, b);
// }

//  TEST(MySuite, HUH) {
//      int a = 5;
//      int b = 2;
//      ASSERT_LT(a, b);
// }
