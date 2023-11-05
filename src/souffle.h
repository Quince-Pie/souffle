#ifndef SOUFFLE_H
#define SOUFFLE_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int len;
    char *fail_msg;
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

// Intentional Fail with a message
#define ASSERT_FAIL(fmt, ...)                                                                      \
    do {                                                                                           \
        status_info->status = Fail;                                                                \
        LOG_FAIL(fmt, ##__VA_ARGS__);                                                              \
        return;                                                                                    \
    } while (0)

#define ASSERT_TRUE(cond)                                                                          \
    do {                                                                                           \
        if (!cond) {                                                                               \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"true\"\n\t  >> Got: \"%s\"", #cond);                    \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_FALSE(cond)                                                                         \
    do {                                                                                           \
        if (cond) {                                                                                \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"false\"\n\t  >> Got: \"%s\"", #cond);                   \
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

#define ASSERT_PTR_EQ(a, b)                                                                        \
    do {                                                                                           \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a != b) {                                                                              \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"%p\"\n\t  >> Got: \"%p\"", (void *)a, (void *)b);       \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_PTR_NE(a, b)                                                                        \
    do {                                                                                           \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a == b) {                                                                              \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"%p\"\n\t  >> Got: \"%p\"", (void *)a, (void *)b);       \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_NULL(a)                                                                             \
    do {                                                                                           \
        if (a != NULL) {                                                                           \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"NULL\"\n\t  >> Got: \"%p\"", (void *)a);                \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_NOT_NULL(a)                                                                         \
    do {                                                                                           \
        if (a == NULL) {                                                                           \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"NOT NULL\"\n\t  >> Got: \"NULL\"");                     \
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

#define ASSERT_LTE(a, b)                                                                           \
    do {                                                                                           \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a > b) {                                                                               \
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

#define ASSERT_GTE(a, b)                                                                           \
    do {                                                                                           \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a < b) {                                                                               \
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
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
        if (status_info->status == Fail) {                                                         \
            status_info->fail_msg = malloc(64 + size * 4);                                         \
            LOG_FAIL("\n\t  >> Expected: [ %d", arr1[0]);                                          \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_FAIL(", %d", arr1[i]);                                                         \
            }                                                                                      \
            LOG_FAIL(" ]");                                                                        \
            LOG_FAIL("\n\t  >> Got: [ %d", arr2[0]);                                               \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_FAIL(", %d", arr2[i]);                                                         \
            }                                                                                      \
            LOG_FAIL(" ]");                                                                        \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_ARR_NE(arr1, arr2, size)                                                            \
    do {                                                                                           \
        static_assert(_Generic((arr1[0]), typeof(arr2[0]): 1, default: 0),                         \
                      "Element type mismatch");                                                    \
        for (typeof(size) i = 0; i < size; ++i) {                                                  \
            if (((arr1)[i] == (arr2)[i])) {                                                        \
                status_info->status = Fail;                                                        \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
        if (status_info->status == Fail) {                                                         \
            status_info->fail_msg = malloc(64 + size * 4);                                         \
            LOG_FAIL("\n\t  >> Expected: [ %d", arr1[0]);                                          \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_FAIL(", %d", arr1[i]);                                                         \
            }                                                                                      \
            LOG_FAIL(" ]");                                                                        \
            LOG_FAIL("\n\t  >> Got: [ %d", arr2[0]);                                               \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_FAIL(", %d", arr2[i]);                                                         \
            }                                                                                      \
            LOG_FAIL(" ]");                                                                        \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_STR_EQ(str1, str2)                                                                  \
    do {                                                                                           \
        if (strcmp(str1, str2) != 0) {                                                             \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"%s\"\n\t  >> Got: \"%s\"", str1, str2);                 \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_STR_NE(str1, str2)                                                                  \
    do {                                                                                           \
        if (strcmp(str1, str2) == 0) {                                                             \
            status_info->status = Fail;                                                            \
            LOG_FAIL("\n\t  >> Expected: \"%s\"\n\t  >> Got: \"%s\"", str1, str2);                 \
            return;                                                                                \
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

int run_all_tests();

#define SETUP(suite, name)                                                                         \
    extern void suite##_##name##_setup(void **ctx) __attribute__((weak));                          \
    void suite##_##name##_setup(void **ctx)

#define TEARDOWN(suite, name)                                                                      \
    extern void suite##_##name##_teardown(void **ctx) __attribute__((weak));                       \
    void suite##_##name##_teardown(void **ctx)

#define TEST(suite, name)                                                                          \
    SETUP(suite, name);                                                                            \
    TEARDOWN(suite, name);                                                                         \
    void suite##_##name([[maybe_unused]] StatusInfo *status_info, [[maybe_unused]] void **ctx);    \
    __attribute__((constructor)) void reg_##suite##_##name() {                                     \
        register_test(#suite, #name, suite##_##name, suite##_##name##_setup,                       \
                      suite##_##name##_teardown);                                                  \
    }                                                                                              \
    void suite##_##name([[maybe_unused]] StatusInfo *status_info, [[maybe_unused]] void **ctx)

#endif // SOUFFLE_H
