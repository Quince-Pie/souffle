#ifndef SOUFFLE_H
#define SOUFFLE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum Status {
    Success,
    Fail,
    Skip,
    Timeout,
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
    ({                                                                                             \
        status_info->status = Skip;                                                                \
        return;                                                                                    \
    })

#define ISUNSIGNED(x) ((typeof(x))0 - 1 > 0)

#define ISFLOAT(x) _Generic((x), float: true, double: true, long double: true, default: false)
// ---------------- ASSERTIONS ----------------

#define ASSERT_TRUE(cond)                                                                          \
    ({                                                                                             \
        if (!cond) {                                                                               \
            status_info->status = Fail;                                                            \
            LOG_FAIL("Expected: \"true\", got: \"%s\"", #cond);                                    \
            return;                                                                                \
        }                                                                                          \
    })

#define ASSERT_EQ(a, b)                                                                            \
    ({                                                                                             \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a != b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_FAIL("Expected: \"%Lf\", got: \"%Lf\"", (long double)a, (long double)b);       \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_FAIL("Expected: \"%zu\", got: \"%zu\"", (uintmax_t)a, (uintmax_t)b);           \
            } else {                                                                               \
                LOG_FAIL("Expected: \"%zd\", got: \"%zd\"", (intmax_t)a, (intmax_t)b);             \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    })

#define ASSERT_NE(a, b)                                                                            \
    ({                                                                                             \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a == b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_FAIL("Expected: \"%Lf\", got: \"%Lf\"", (long double)a, (long double)b);       \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_FAIL("Expected: \"%zu\", got: \"%zu\"", (uintmax_t)a, (uintmax_t)b);           \
            } else {                                                                               \
                LOG_FAIL("Expected: \"%zd\", got: \"%zd\"", (intmax_t)a, (intmax_t)b);             \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    })

#define ASSERT_LT(a, b)                                                                            \
    ({                                                                                             \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a >= b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_FAIL("Expected: \"%Lf\", got: \"%Lf\"", (long double)a, (long double)b);       \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_FAIL("Expected: \"%zu\", got: \"%zu\"", (uintmax_t)a, (uintmax_t)b);           \
            } else {                                                                               \
                LOG_FAIL("Expected: \"%zd\", got: \"%zd\"", (intmax_t)a, (intmax_t)b);             \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    })

#define ASSERT_GT(a, b)                                                                            \
    ({                                                                                             \
        static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch");                   \
        if (a <= b) {                                                                              \
            status_info->status = Fail;                                                            \
            if (ISFLOAT(a)) {                                                                      \
                LOG_FAIL("Expected: \"%Lf\", got: \"%Lf\"", (long double)a, (long double)b);       \
            } else if (ISUNSIGNED(a)) {                                                            \
                LOG_FAIL("Expected: \"%zu\", got: \"%zu\"", (uintmax_t)a, (uintmax_t)b);           \
            } else {                                                                               \
                LOG_FAIL("Expected: \"%zd\", got: \"%zd\"", (intmax_t)a, (intmax_t)b);             \
            }                                                                                      \
            return;                                                                                \
        }                                                                                          \
    })

#define ASSERT_ARR_EQ(arr1, arr2, size)                                                            \
    ({                                                                                             \
        static_assert(_Generic((arr1[0]), typeof(arr2[0]): 1, default: 0),                         \
                      "Element type mismatch");                                                    \
        for (typeof(size) i = 0; i < size; ++i) {                                                  \
            if (((arr1)[i] != (arr2)[i])) {                                                        \
                status_info->status = Fail;                                                        \
                LOG_FAIL("Expected: \"%d\", got: \"%d\" in Idx: %d", a[i], b[i], i);               \
                return;                                                                            \
            }                                                                                      \
        }                                                                                          \
    })

#define ASSERT_ARR_NE(arr1, arr2, size)                                                            \
    ({                                                                                             \
        static_assert(_Generic((arr1[0]), typeof(arr2[0]): 1, default: 0),                         \
                      "Element type mismatch");                                                    \
        for (typeof(size) i = 0; i < size; ++i) {                                                  \
            if (((arr1)[i] == (arr2)[i])) {                                                        \
                status_info->status = Fail;                                                        \
                LOG_FAIL("Expected: \"%d\", got: \"%d\" in Idx: %d", a[i], b[i], i);               \
                return;                                                                            \
            }                                                                                      \
        }                                                                                          \
    })

// -------------- ASSERTIONS END --------------

typedef void (*TestFunc)(StatusInfo *status_info);

typedef struct Test {
    const char *name;
    TestFunc func;
} Test;

typedef struct TestsVec {
    Test *tests;
    size_t capacity;
    size_t len;
} TestsVec;

void register_test(const char *suite, const char *name, TestFunc func);
void run_all_tests();

#define TEST(suite, name)                                                                          \
    void suite##_##name(StatusInfo *status_info);                                                  \
    __attribute__((constructor)) void reg_##suite##_##name() {                                     \
        register_test(#suite, #name, suite##_##name);                                              \
    }                                                                                              \
    void suite##_##name(StatusInfo *status_info)

#endif // SOUFFLE_H
