#ifndef SOUFFLE_H
#define SOUFFLE_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// macro for typeof
#if defined(__GNUC__) || defined(__clang__)
#define typeof __typeof__
#endif

enum Status {
    Success,
    Fail,
    Skip,
    Timeout,
    // Only for Windows since the tests run in a separate thread.
    Crashed,
};

typedef struct SouffleString {
    char *buf;
    size_t capacity;
    size_t len;
} SouffleString;

typedef struct StatusInfo {
    enum Status status;
    SouffleString *msg;
} StatusInfo;

// Utility macro: Make sure the function is only used the same way as printf
#define PRINTF(x) __attribute__((__format__(__printf__, (x), (x + 1))))

void
souffle_log_msg(StatusInfo *status_info, const char *file, int lineno, const char *fmt, ...)
    PRINTF(4);

void
souffle_log_msg_raw(StatusInfo *status_info, const char *fmt, ...) PRINTF(2);

#define LOG_TRACE_MSG(fmt, ...)                                                                    \
    do {                                                                                           \
        souffle_log_msg(status_info, __FILE__, __LINE__, fmt, ##__VA_ARGS__);                      \
    } while (0)

#define LOG_MSG(fmt, ...)                                                                          \
    do {                                                                                           \
        souffle_log_msg_raw(status_info, fmt, ##__VA_ARGS__);                                      \
    } while (0)

#define SKIP_TEST()                                                                                \
    do {                                                                                           \
        status_info->status = Skip;                                                                \
        return;                                                                                    \
    } while (0)

#define FAIL_TEST()                                                                                \
    do {                                                                                           \
        status_info->status = Fail;                                                                \
        return;                                                                                    \
    } while (0)

#define ISFLOAT(x) _Generic((x), float: true, double: true, long double: true, default: false)
#define ISUNSIGNED(x)                                                                              \
    _Generic((x),                                                                                  \
        unsigned char: true,                                                                       \
        unsigned short: true,                                                                      \
        unsigned int: true,                                                                        \
        unsigned long: true,                                                                       \
        unsigned long long: true,                                                                  \
        default: false)
// ---------------- ASSERTIONS ----------------

#define ASSERT_TRUE(cond)                                                                          \
    do {                                                                                           \
        if (!cond) {                                                                               \
            status_info->status = Fail;                                                            \
            LOG_TRACE_MSG("Expected: \"true\"\n\t  >> Got: \"false\"\n");                          \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_FALSE(cond)                                                                         \
    do {                                                                                           \
        if (cond) {                                                                                \
            status_info->status = Fail;                                                            \
            LOG_TRACE_MSG("Expected: \"false\"\n\t  >> Got: \"true\"\n");                          \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_EQ(a, b)                                                                            \
    do {                                                                                           \
        if (a != b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_TRACE_MSG("Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"\n", (long double)a,          \
                              (long double)b);                                                     \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_TRACE_MSG("Expected: \"%zu\"\n\t  >> Got: \"%zu\"\n", (uintmax_t)a,            \
                              (uintmax_t)b);                                                       \
            } else {                                                                               \
                LOG_TRACE_MSG("Expected: \"%zd\"\n\t  >> Got: \"%zd\"\n", (intmax_t)a,             \
                              (intmax_t)b);                                                        \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_PTR_EQ(a, b)                                                                        \
    do {                                                                                           \
        if (a != b) {                                                                              \
            status_info->status = Fail;                                                            \
            LOG_TRACE_MSG("Expected: \"%p\"\n\t  >> Got: \"%p\"\n", (void *)a, (void *)b);         \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_PTR_NE(a, b)                                                                        \
    do {                                                                                           \
        if (a == b) {                                                                              \
            status_info->status = Fail;                                                            \
            LOG_TRACE_MSG("Expected: \"%p\"\n\t  >> Got: \"%p\"\n", (void *)a, (void *)b);         \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_NULL(a)                                                                             \
    do {                                                                                           \
        if (a != NULL) {                                                                           \
            status_info->status = Fail;                                                            \
            LOG_TRACE_MSG("Expected: \"NULL\"\n\t  >> Got: \"%p\"\n", (void *)a);                  \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_NOT_NULL(a)                                                                         \
    do {                                                                                           \
        if (a == NULL) {                                                                           \
            status_info->status = Fail;                                                            \
            LOG_TRACE_MSG("Expected: \"NOT NULL\"\n\t  >> Got: \"NULL\"\n");                       \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_NE(a, b)                                                                            \
    do {                                                                                           \
        if (a == b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_TRACE_MSG("Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"\n", (long double)a,          \
                              (long double)b);                                                     \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_TRACE_MSG("Expected: \"%zu\"\n\t  >> Got: \"%zu\"\n", (uintmax_t)a,            \
                              (uintmax_t)b);                                                       \
            } else {                                                                               \
                LOG_TRACE_MSG("Expected: \"%zd\"\n\t  >> Got: \"%zd\"\n", (intmax_t)a,             \
                              (intmax_t)b);                                                        \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_LT(a, b)                                                                            \
    do {                                                                                           \
        if (a >= b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_TRACE_MSG("Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"\n", (long double)a,          \
                              (long double)b);                                                     \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_TRACE_MSG("Expected: \"%zu\"\n\t  >> Got: \"%zu\"\n", (uintmax_t)a,            \
                              (uintmax_t)b);                                                       \
            } else {                                                                               \
                LOG_TRACE_MSG("Expected: \"%zd\"\n\t  >> Got: \"%zd\"\n", (intmax_t)a,             \
                              (intmax_t)b);                                                        \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_LTE(a, b)                                                                           \
    do {                                                                                           \
        if (a > b) {                                                                               \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_TRACE_MSG("Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"\n", (long double)a,          \
                              (long double)b);                                                     \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_TRACE_MSG("Expected: \"%zu\"\n\t  >> Got: \"%zu\"\n", (uintmax_t)a,            \
                              (uintmax_t)b);                                                       \
            } else {                                                                               \
                LOG_TRACE_MSG("Expected: \"%zd\"\n\t  >> Got: \"%zd\"\n", (intmax_t)a,             \
                              (intmax_t)b);                                                        \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_GT(a, b)                                                                            \
    do {                                                                                           \
        if (a <= b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_TRACE_MSG("Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"\n", (long double)a,          \
                              (long double)b);                                                     \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_TRACE_MSG("Expected: \"%zu\"\n\t  >> Got: \"%zu\"\n", (uintmax_t)a,            \
                              (uintmax_t)b);                                                       \
            } else {                                                                               \
                LOG_TRACE_MSG("Expected: \"%zd\"\n\t  >> Got: \"%zd\"\n", (intmax_t)a,             \
                              (intmax_t)b);                                                        \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_GTE(a, b)                                                                           \
    do {                                                                                           \
        if (a < b) {                                                                               \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_TRACE_MSG("Expected: \"%Lf\"\n\t  >> Got: \"%Lf\"\n", (long double)a,          \
                              (long double)b);                                                     \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_TRACE_MSG("Expected: \"%zu\"\n\t  >> Got: \"%zu\"\n", (uintmax_t)a,            \
                              (uintmax_t)b);                                                       \
            } else {                                                                               \
                LOG_TRACE_MSG("Expected: \"%zd\"\n\t  >> Got: \"%zd\"\n", (intmax_t)a,             \
                              (intmax_t)b);                                                        \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_STR_EQ(str1, str2)                                                                  \
    do {                                                                                           \
        if (strcmp(str1, str2) != 0) {                                                             \
            status_info->status = Fail;                                                            \
            LOG_TRACE_MSG("Expected: \"%s\"\n\t  >> Got: \"%s\"\n", str1, str2);                   \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_STR_NE(str1, str2)                                                                  \
    do {                                                                                           \
        if (strcmp(str1, str2) == 0) {                                                             \
            status_info->status = Fail;                                                            \
            LOG_TRACE_MSG("Expected: \"%s\"\n\t  >> Got: \"%s\"\n", str1, str2);                   \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_INT_ARR_EQ(arr1, arr2, size)                                                        \
    do {                                                                                           \
        for (typeof(size) i = 0; i < size; ++i) {                                                  \
            if (((arr1)[i] != (arr2)[i])) {                                                        \
                status_info->status = Fail;                                                        \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
        if (status_info->status == Fail) {                                                         \
            LOG_TRACE_MSG("Expected: [ %zd", (intmax_t)arr1[0]);                                   \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_MSG(", %zd", (intmax_t)arr1[i]);                                               \
            }                                                                                      \
            LOG_MSG(" ]");                                                                         \
            LOG_MSG("\n\t  >> Got: [ %zd", (intmax_t)arr2[0]);                                     \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_MSG(", %zd", (intmax_t)arr2[i]);                                               \
            }                                                                                      \
            LOG_MSG(" ]\n");                                                                       \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_UINT_ARR_EQ(arr1, arr2, size)                                                       \
    do {                                                                                           \
        for (typeof(size) i = 0; i < size; ++i) {                                                  \
            if (((arr1)[i] != (arr2)[i])) {                                                        \
                status_info->status = Fail;                                                        \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
        if (status_info->status == Fail) {                                                         \
            LOG_TRACE_MSG("Expected: [ %d", (uintmax_t)arr1[0]);                                   \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_MSG(", %zu", (uintmax_t)arr1[i]);                                              \
            }                                                                                      \
            LOG_MSG(" ]");                                                                         \
            LOG_MSG("\n\t  >> Got: [ %zu", (uintmax_t)arr2[0]);                                    \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_MSG(", %zu", (uintmax_t)arr2[i]);                                              \
            }                                                                                      \
            LOG_MSG(" ]\n");                                                                       \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_FLOAT_ARR_EQ(arr1, arr2, size)                                                      \
    do {                                                                                           \
        for (typeof(size) i = 0; i < size; ++i) {                                                  \
            if (((arr1)[i] != (arr2)[i])) {                                                        \
                status_info->status = Fail;                                                        \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
        if (status_info->status == Fail) {                                                         \
            LOG_TRACE_MSG("Expected: [ %Lf", (long double)arr1[0]);                                \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_MSG(", %Lf", (long double)arr1[i]);                                            \
            }                                                                                      \
            LOG_MSG(" ]");                                                                         \
            LOG_MSG("\n\t  >> Got: [ %Lf", (long double)arr2[0]);                                  \
            for (typeof(size) i = 1; i < size; ++i) {                                              \
                LOG_MSG(", %Lf", (long double)arr2[i]);                                            \
            }                                                                                      \
            LOG_MSG(" ]\n");                                                                       \
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

void
register_test(const char *suite, const char *name, TestFunc func, SetupFunc setup,
              TeardownFunc teardown);

int
run_all_tests();

#define SETUP(suite, name) __attribute__((weak)) void suite##_##name##_setup(void **ctx)

#define TEARDOWN(suite, name) __attribute__((weak)) void suite##_##name##_teardown(void **ctx)

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
