#ifndef SOUFFLE_H
#define SOUFFLE_H

#include<stdio.h>
#include<stdlib.h>


enum Status {
    Success,
    Fail,
    Skip,
};

// Utility macro: Make sure the function is only used the same way as printf
#define PRINTF(x) __attribute__((__format__(__printf__, (x), (x + 1))))

void err_print(const char *file, int lineno, const char *fmt, ...) PRINTF(3);


#define LOG_FAIL(fmt, ...)                                \
  err_print(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

// ---------------- ASSERTIONS ---------------- 

#define ASSERT_TRUE(cond) ({                                \
    if (!cond) {                                            \
        LOG_FAIL("Expected: \"true\", got: \"%s\"", #cond); \
        *status = Fail;                                     \
        return;                                             \
    }                                                       \
})

#define ASSERT_EQ(a, b) ({                                                   \
    static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch"); \
    if (a != b) {                                                            \
        LOG_FAIL("Expected: \"%d\", got: \"%d\"", a, b);                     \
        *status = Fail;                                                      \
        return;                                                              \
    }                                                                        \
})

#define ASSERT_NE(a, b) ({                                                   \
    static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch"); \
    if (a == b) {                                                            \
        LOG_FAIL("Expected: \"%d\", got: \"%d\"", a, b);                     \
        *status = Fail;                                                      \
        return;                                                              \
    }                                                                        \
})

#define ASSERT_LT(a, b) ({                                                   \
    static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch"); \
    if (a >= b) {                                                            \
        LOG_FAIL("Expected: \"%d\", got: \"%d\"", a, b);                     \
        *status = Fail;                                                      \
        return;                                                              \
    }                                                                        \
})

#define ASSERT_GT(a, b) ({                                                   \
    static_assert(_Generic((a), typeof(b): 1, default: 0), "Type mismatch"); \
    if (a <= b) {                                                            \
        LOG_FAIL("Expected: \"%d\", got: \"%d\"", a, b);                     \
        *status = Fail;                                                      \
        return;                                                              \
    }                                                                        \
})

#define ASSERT_ARR_EQ(arr1, arr2, size) ({                                                        \
    static_assert(_Generic((arr1[0]), typeof(arr2[0]): 1, default: 0), "Element type mismatch");  \
    for (typeof(size) i = 0; i < size; ++i) {                                                     \
        if (((arr1)[i] != (arr2)[i])) {                                                           \
            LOG_FAIL("Expected: \"%d\", got: \"%d\" in Idx: %d", a[i], b[i], i);                  \
            *status = Fail;                                                                       \
            return;                                                                               \
        }                                                                                         \
    }                                                                                             \
})

// -------------- ASSERTIONS END --------------


typedef void (*TestFunc)(enum Status *status);

typedef struct Test {
  const char* name;
  TestFunc func;  
} Test;

typedef struct TestsVec {
    Test *tests;    
    size_t capacity;
    size_t len;
} TestsVec;


void register_test(const char *suite, const char *name, TestFunc func);
void run_all_tests();

#define TEST(suite, name)                                      \
    void suite##_##name(enum Status *status);                  \
    __attribute__((constructor)) void reg_##suite##_##name() { \
        register_test(#suite, #name, suite##_##name);          \
    }                                                          \
    void suite##_##name(enum Status *status)



#endif // SOUFFLE_H
