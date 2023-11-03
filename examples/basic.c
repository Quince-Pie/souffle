#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#include <stdbool.h>
#endif
// !!!! The only needed header here. !!!!
#include "../src/souffle.h"
SETUP(MySuite, TestCase1) {
    int *data = malloc(sizeof(int));
    assert(data);
    *data = 5;
    *ctx = data;
}
TEST(MySuite, TestCase1) {
    ASSERT_TRUE(true);
    int *data = (int *)*ctx;
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*data, 5);
}
TEARDOWN(MySuite, TestCase1) { free(*ctx); }

TEST(MySuite, test_number_eq) {
    int a = 5;
    int b = 1;
    ASSERT_EQ(a, b);
}

TEST(MySuite, fffff) {
// ASSERT_EQ(12, 1);
#ifndef _WIN32
    raise(SIGSEGV);
#else
    RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
#endif
}

TEST(MySuite, pass) { ASSERT_EQ(1, 1); }

TEST(MySuite, pass_fail_pass) {
    ASSERT_EQ(1, 1);
    ASSERT_EQ(2, 1);
    ASSERT_EQ(1, 1);
}

TEST(MySuite, float_check) { ASSERT_EQ(1.5, 2.5); }

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

// Used to test print border expansion.
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
TEST(MySuite, timeoutf) {
    for (size_t i = 0; i < 3; i++) {
#ifndef _WIN32
        sleep(1);
#else
        Sleep(1000);
#endif
        ASSERT_EQ(i, i);
    }
}

TEST(MySuite, timeout) {
    for (size_t i = 0; i < 10000000000; i++) {
#ifndef _WIN32
        sleep(1);
#else
        Sleep(1000);
#endif
        ASSERT_EQ(i, i);
    }
}

TEST(huh, 1) { ASSERT_TRUE(true); }
