#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// The only needed header here.
#include "../souffle.h"

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

TEST(MySuite, pass) { ASSERT_EQ(1, 1); }

TEST(MySuite, pass_fail_pass) {
    ASSERT_EQ(1, 1);
    ASSERT_EQ(2, 1);
    ASSERT_EQ(1, 1);
}

TEST(MySuite, float_check) {
    ASSERT_EQ(1.5, 2.5);
}

TEST(MySuite, pass_fail) {
    ASSERT_EQ(1, 1);
    ASSERT_EQ(2, 1);
}

TEST(MySuite, pass_crash) {
    ASSERT_EQ(1, 1);
    // raise(SIGSEGV);
}

TEST(MySuite, skip_me) {
    SKIP_TEST();
    ASSERT_EQ(1, 1);
}

TEST(
    EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE,
    EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE) {
    ASSERT_EQ(1, 1);
}

TEST(arr, arr1) {
    int a[3] = {1, 2, 3};
    int b[3] = {1, 2, 3};
    ASSERT_ARR_EQ(a, b, 3);
}

TEST(MySuite, timeout) {
    for (size_t i = 0; i < 10000000000; i++) {
        sleep(1);
        ASSERT_EQ(i, i);
    }
}
