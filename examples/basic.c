#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#include <stdbool.h>
#endif
// !!!! The only needed header here. !!!!
#include "../src/souffle.h"

SETUP(main_suite, TestCase1) {
    int *data = malloc(sizeof(int));
    assert(data);
    *data = 5;
    *ctx = data;
}

TEST(main_suite, TestCase1) {
    ASSERT_TRUE(true);
    int *data = (int *)*ctx;
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(*data, 5);
}

TEARDOWN(main_suite, TestCase1) { free(*ctx); }

TEST(main_suite, test_number_eq) {
    int a = 5;
    int b = 1;
    ASSERT_EQ(a, b);
}

TEST(main_suite, exception_test) {
// ASSERT_EQ(12, 1);
#ifndef _WIN32
    raise(SIGSEGV);
#else
    RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
#endif
}

TEST(main_suite, pass) { ASSERT_EQ(1, 1); }

TEST(main_suite, pass_fail_pass) {
    ASSERT_EQ(1, 1);
    ASSERT_EQ(2, 1);
    ASSERT_EQ(1, 1);
}

TEST(main_suite, float_check) { ASSERT_EQ(1.5, 2.5); }

TEST(main_suite, pass_fail) {
    ASSERT_EQ(1, 1);
    ASSERT_EQ(2, 1);
}

TEST(main_suite, skip_me) {
    SKIP_TEST();
    ASSERT_EQ(1, 1);
}

// Used to test print border expansion.
TEST(
    EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE,
    EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE) {
    ASSERT_EQ(1, 1);
}

TEST(arr_suite, array_check) {
    int a[3] = {1, 2, 3};
    int b[3] = {1, 4, 3};
    ASSERT_INT_ARR_EQ(a, b, 3);
}

TEST(main_suite, long_test) {
    for (size_t i = 0; i < 3; i++) {
#ifndef _WIN32
        sleep(1);
#else
        Sleep(1000);
#endif
        ASSERT_EQ(i, i);
    }
}

TEST(main_suite, timeout_test) {
    for (size_t i = 0; i < 10000000000; i++) {
#ifndef _WIN32
        sleep(1);
#else
        Sleep(1000);
#endif
        ASSERT_EQ(i, i);
    }
}

TEST(suite_2, is_true) { ASSERT_TRUE(true); }

TEST(main_suite, string_test) { ASSERT_STR_EQ("Hello, World", "Hello World!"); }

TEST(main_suite, log_on_pass) {
    ASSERT_EQ(1, 1);
    LOG_MSG("\t  This is a log message\n");
}
