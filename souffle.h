#ifndef SOUFFLE_H
#define SOUFFLE_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum Status {
    Success,
    Fail,
    Skip,
    Timeout,
    // Only for Windows since the tests run in a separate thread.
    Crashed,
};

typedef struct StatusInfo {
    enum Status status;
    char fail_msg[128];
} StatusInfo;

// Utility macro: Make sure the function is only used the same way as printf
#define PRINTF(x) __attribute__((__format__(__printf__, (x), (x + 1))))

void err_print(StatusInfo *status_info, const char *file, int lineno, const char *fmt, ...)
    PRINTF(4);

#define LOG_FAIL(fmt, ...) err_print(status_info, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define SKIP_TEST()                                                                                \
    do {                                                                                           \
        status_info->status = Skip;                                                                \
        return;                                                                                    \
    } while (0)

#define ISUNSIGNED(x) ((typeof(x))0 - 1 > 0)

#define ISFLOAT(x) _Generic((x), float: true, double: true, long double: true, default: false)
// ---------------- ASSERTIONS ----------------

#define ASSERT_TRUE(cond)                                                                          \
    do {                                                                                           \
        if (!cond) {                                                                               \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"true\"\n\t  >> Got: \"%s\"", #cond);                    \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_EQ(a, b)                                                                            \
    do {                                                                                           \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a != b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_FAIL("\n\t  >> Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"", (long double)a,        \
                         (long double)b);                                                          \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_FAIL("\n\t  >> Expected: \"%zu\"\n\t  >> Got: \"%zu\"", (uintmax_t)a,          \
                         (uintmax_t)b);                                                            \
            } else {                                                                               \
                LOG_FAIL("\n\t  >> Expected: \"%zd\"\n\t  >> Got: \"%zd\"", (intmax_t)a,           \
                         (intmax_t)b);                                                             \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_NULL(a)                                                                             \
    do {                                                                                           \
        if (a != NULL) {                                                                           \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"NULL\"\n\t  >> Got: \"%p\"", a);                        \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_NOT_NULL(a)                                                                         \
    do {                                                                                           \
        if (a == NULL) {                                                                           \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"not NULL\"\n\t  >> Got: \"%p\"", a);                    \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_NE(a, b)                                                                            \
    do {                                                                                           \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a == b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_FAIL("\n\t  >> Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"", (long double)a,        \
                         (long double)b);                                                          \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_FAIL("\n\t  >> Expected: \"%zu\"\n\t  >> Got: \"%zu\"", (uintmax_t)a,          \
                         (uintmax_t)b);                                                            \
            } else {                                                                               \
                LOG_FAIL("\n\t  >> Expected: \"%zd\"\n\t  >> Got: \"%zd\"", (intmax_t)a,           \
                         (intmax_t)b);                                                             \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_LT(a, b)                                                                            \
    do {                                                                                           \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a >= b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_FAIL("\n\t  >> Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"", (long double)a,        \
                         (long double)b);                                                          \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_FAIL("\n\t  >> Expected: \"%zu\"\n\t  >> Got: \"%zu\"", (uintmax_t)a,          \
                         (uintmax_t)b);                                                            \
            } else {                                                                               \
                LOG_FAIL("\n\t  >> Expected: \"%zd\"\n\t  >> Got: \"%zd\"", (intmax_t)a,           \
                         (intmax_t)b);                                                             \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_GT(a, b)                                                                            \
    do {                                                                                           \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a <= b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_FAIL("\n\t  >> Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"", (long double)a,        \
                         (long double)b);                                                          \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_FAIL("\n\t  >> Expected: \"%zu\"\n\t  >> Got: \"%zu\"", (uintmax_t)a,          \
                         (uintmax_t)b);                                                            \
            } else {                                                                               \
                LOG_FAIL("\n\t  >> Expected: \"%zd\"\n\t  >> Got: \"%zd\"", (intmax_t)a,           \
                         (intmax_t)b);                                                             \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_ARR_EQ(arr1, arr2, size)                                                            \
    do {                                                                                           \
        static_assert(_Generic((arr1[0]), typeof(arr2[0]): 1, default: 0),                         \
                      "Element type mismatch");                                                    \
        for (typeof(size) i = 0; i < size; ++i) {                                                  \
            if (((arr1)[i] != (arr2)[i])) {                                                        \
                status_info->status = Fail;                                                        \
                LOG_FAIL("\n\t  >> Expected: \"%d\"\n\t  >> Got: \"%d\" in Idx: %d", a[i], b[i],   \
                         i);                                                                       \
                return;                                                                            \
            }                                                                                      \
        }                                                                                          \
    } while (0)

#define ASSERT_ARR_NE(arr1, arr2, size)                                                            \
    do {                                                                                           \
        static_assert(_Generic((arr1[0]), typeof(arr2[0]): 1, default: 0),                         \
                      "Element type mismatch");                                                    \
        for (typeof(size) i = 0; i < size; ++i) {                                                  \
            if (((arr1)[i] == (arr2)[i])) {                                                        \
                status_info->status = Fail;                                                        \
                LOG_FAIL("\n\t  >> Expected: \"%d\"\n\t  >> Got: \"%d\" in Idx: %d", a[i], b[i],   \
                         i);                                                                       \
                return;                                                                            \
            }                                                                                      \
        }                                                                                          \
    } while (0)

// -------------- ASSERTIONS END --------------

typedef void (*TestFunc)(StatusInfo *status_info, void **ctx);

typedef void (*SetupFunc)(void **ctx);

typedef void (*TeardownFunc)(void **ctx);

typedef struct Test {
    const char *name;
    TestFunc func;
    SetupFunc setup;
    TeardownFunc teardown;
} Test;

typedef struct TestsVec {
    Test *tests;
    size_t capacity;
    size_t len;
} TestsVec;

void register_test(const char *suite, const char *name, TestFunc func, SetupFunc setup,
                   TeardownFunc teardown);

void run_all_tests();

#define SETUP(suite, name)                                                                         \
    extern void suite##_##name##_setup(void **ctx) __attribute__((weak));                          \
    void suite##_##name##_setup(void **ctx)

#define TEARDOWN(suite, name)                                                                      \
    extern void suite##_##name##_teardown(void **ctx) __attribute__((weak));                       \
    void suite##_##name##_teardown(void **ctx)

#define TEST(suite, name)                                                                          \
    SETUP(suite, name);                                                                            \
    TEARDOWN(suite, name);                                                                         \
    void suite##_##name(StatusInfo *status_info, void **ctx);                                      \
    __attribute__((constructor)) void reg_##suite##_##name() {                                     \
        register_test(#suite, #name, suite##_##name, suite##_##name##_setup,                       \
                      suite##_##name##_teardown);                                                  \
    }                                                                                              \
    void suite##_##name(StatusInfo *status_info, void **ctx)

#endif // SOUFFLE_H
