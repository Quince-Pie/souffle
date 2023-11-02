#define _POSIX_C_SOURCE 200809L
#include "souffle.h"
#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>
#include "klib/khash.h"
#include "stdlib.h"

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define MAGENTA "\033[35m"
#define YELLOW "\033[0;33m"
#define GREY "\033[90m"
#define RESET "\033[0m"

// Separator
static const char *DASHES =
    "-------------------------------------------------------------------------------";

typedef struct String {
    char *buf;
    size_t capacity;
    size_t len;
} String;

static String *string_init() {
    String *str = malloc(sizeof(String));
    assert(str);
    str->buf = malloc(128 * sizeof(char));
    str->capacity = 128;
    str->len = 0;
    return str;
}

static void string_free(String *str) {
    free(str->buf);
    free(str);
}
static void string_rewind(String *str) {
    str->len = 0;
    str->buf[0] = '\0';
}

void string_append(String *str, const char *fmt, ...) PRINTF(2);
void string_append(String *str, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int size_needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    va_start(args, fmt);
    if (size_needed > str->capacity - str->len) {
        str->buf = realloc(str->buf, (size_t)(str->capacity * 2) * sizeof(char));
        assert(str->buf);
        str->capacity *= 2;
    }
    str->len += vsnprintf(str->buf + str->len, str->capacity, fmt, args);
    // Cleanup
    va_end(args);
}

KHASH_MAP_INIT_STR(str_map, TestsVec *)
khash_t(str_map) * test_suites;

static size_t tcount = 0;
static int largest_test_name = 0;
static int largest_suite_name = 0;

static TestsVec *test_vec_init() {
    TestsVec *tv = malloc(sizeof(TestsVec));
    assert(tv);
    tv->tests = malloc(128 * sizeof(Test));
    tv->capacity = 128;
    tv->len = 0;
    return tv;
}

static void test_vec_free(TestsVec *tv) {
    free(tv->tests);
    free(tv);
}

static void test_vec_push(TestsVec *tv, Test t) {
    if (tv->len == tv->capacity) {
        tv->tests = realloc(tv->tests, (size_t)(tv->capacity * 2) * sizeof(Test));
        assert(tv->tests);
        tv->capacity *= 2;
    }
    tv->tests[tv->len] = t;
    tv->len++;
    return;
}

void err_print(StatusInfo *status_info, const char *file, int lineno, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int idx = vsnprintf(status_info->fail_msg, 128, fmt, args);
    idx += snprintf((status_info->fail_msg) + idx, 128, " in [%s:%d]", file, lineno);
    // Cleanup
    va_end(args);
}

void register_test(const char *suite, const char *name, TestFunc func) {
    if (test_suites == NULL) {
        test_suites = kh_init(str_map);
    }
    assert(test_suites);
    int ret;
    khiter_t k = kh_put(str_map, test_suites, suite, &ret);
    if (ret == -1) {
        perror("Failed to access hash table");
        exit(EXIT_FAILURE);
    }
    int test_name_len = strlen(name);
    if (test_name_len > largest_test_name) {
        largest_test_name = test_name_len;
    }
    int suite_name_len = strlen(suite);
    if (suite_name_len > largest_suite_name) {
        largest_suite_name = suite_name_len;
    }
    Test t = {
        .func = func,
        .name = name,
    };
    if (ret > 0) { // new key; initialize value to a new vec
        kh_value(test_suites, k) = test_vec_init();
    }
    TestsVec *tv = kh_value(test_suites, k);
    test_vec_push(tv, t);
    tcount += 1;
}

void timeout_handler(int signo) {
    (void)signo;
    exit(Timeout);
}

void alarm_setup() {
    struct sigaction sa;
    sa.sa_handler = timeout_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);
}

void run_all_tests() {
    // Setup Printing End Column
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        w.ws_col = 80;
    }
    int cols = w.ws_col;
    int max_cols = largest_test_name > cols - 31 ? cols - 31 : largest_test_name;

    String *output = string_init();

    assert(test_suites);
    // Result Header
    khint_t scount = kh_size(test_suites);
    time_t t = time(NULL);
    struct tm timeinfo;
    struct tm *tm = gmtime_r(&t, &timeinfo);
    char date[11];
    char time[9];
    strftime(date, sizeof(date), "%Y-%m-%d", tm);
    strftime(time, sizeof(time), "%H:%M:%S", tm);

    string_append(output, "=== Test Run Started ===\n");
    string_append(output, "Date: %s | Time: %s UTC\n", date, time);
    string_append(output, "%s\n\n", DASHES);
    string_append(output, "Running %zu tests in %d suites\n", tcount, scount);
    string_append(output, "%s\n", DASHES);

    int passed = 0;
    int failed = 0;
    int crashed = 0;
    int skipped = 0;
    int timeout = 0;
    khiter_t k;
    for (k = kh_begin(test_suites); k != kh_end(test_suites); ++k) {
        if (kh_exist(test_suites, k)) {
            const char *suite_name = kh_key(test_suites, k);
            TestsVec *tv = kh_val(test_suites, k);
            string_append(output, "\n⣿ Suite: %.*s ⣿\n", max_cols, suite_name);
            for (size_t idx = 0; idx < tv->len; ++idx) {

                int padding = max_cols - strlen(tv->tests[idx].name);
                string_append(output, "  🧪 %.*s ........", max_cols, tv->tests[idx].name);

                for (int i = 0; i < padding; ++i) {
                    string_append(output, ".");
                }
                // setup pipes for transmitting fail info.
                int pipefd[2];
                if (pipe(pipefd) == -1) {
                    perror("Pipe failed");
                    exit(EXIT_FAILURE);
                }
                struct timespec start, end;
                timespec_get(&start, TIME_UTC);
                pid_t pid = fork();
                if (pid == 0) {
                    // child process
                    close(pipefd[0]);
                    alarm_setup();
                    StatusInfo tstatus = {
                        .status = Success,
                        .fail_msg = {0},
                    };
                    alarm(20);
                    tv->tests[idx].func(&tstatus);
                    int wret = write(pipefd[1], tstatus.fail_msg, 128);
                    if (wret == -1) {
                        perror("Failed to write to pipe");
                    }
                    close(pipefd[1]);
                    exit(tstatus.status);
                } else {
                    close(pipefd[1]);
                    // parent process
                    int status;
                    waitpid(pid, &status, 0);
                    timespec_get(&end, TIME_UTC);
                    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 +
                                      (end.tv_nsec - start.tv_nsec) / 1000000;
                    char buf[128] = {0};
                    int rret = read(pipefd[0], buf, 128);
                    if (rret == -1) {
                        perror("Failed to read to pipe");
                    }
                    close(pipefd[0]);
                    if (WIFEXITED(status)) {
                        switch ((enum Status)WEXITSTATUS(status)) {
                        case Success:
                            string_append(output, " " GREEN "[PASSED, %ldms]" RESET "\n",
                                          elapsed_ms);
                            passed += 1;
                            break;
                        case Fail:
                            string_append(output,
                                          " " RED "[FAILED, %ldms]" RESET "\n\t  > Details: %s\n\n",
                                          elapsed_ms, buf);
                            failed += 1;
                            break;
                        case Skip:
                            string_append(output, " " YELLOW "[SKIPPED, ⏭ ]" RESET "\n");
                            skipped += 1;
                            break;
                        case Timeout:
                            string_append(output, " " GREY "[TIMEOUT, ⧖ ]" RESET "\n");
                            timeout += 1;
                            break;
                        default:
                            unreachable();
                        };
                    } else if (WIFSIGNALED(status)) {
                        string_append(output, " " MAGENTA "[CRASHED, ☠ ]" RESET "\n");
                        crashed += 1;
                    }
                    fprintf(stderr, "%s", output->buf);
                    fflush(stderr);
                    string_rewind(output);
                }
            }
            test_vec_free(tv);
        }
    }

    kh_destroy(str_map, test_suites);
    string_append(output, "\n%s\n", DASHES);
    string_append(output, "=== Test Run Summary ===\n");
    string_append(output,
                  "Total Tests: %zu | " GREEN "Passed" RESET ": %d | " RED "Failed" RESET
                  ": %d | " MAGENTA "Crashed" RESET ": %d | " YELLOW "Skipped" RESET ": %d | " GREY
                  "Timeout" RESET ": %d\n",
                  tcount, passed, failed, crashed, skipped, timeout);
    string_append(output, "%s\n", DASHES);
    fprintf(stderr, "%s", output->buf);
    string_free(output);
}

typedef struct TestThread {
    TestsVec *tv;
    int start;
    int end;
    atomic_int *passed;
    atomic_int *failed;
    atomic_int *crashed;
    atomic_int *skipped;
    atomic_int *timeout;
    int max_cols;
} TestThread;

// thread function
int thread_func(void *data) {

    TestThread *tt = (TestThread *)data;
    int max_cols = tt->max_cols;
    TestsVec *tv = tt->tv;
    int start = tt->start;
    int end = tt->end;
    String *output = string_init();

    for (size_t idx = start; idx < end; ++idx) {
        int padding = max_cols - strlen(tv->tests[idx].name);
        string_append(output, "  🧪 %.*s ........", max_cols, tv->tests[idx].name);

        for (int i = 0; i < padding; ++i) {
            string_append(output, ".");
        }
        // setup pipes for transmitting fail info.
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("Pipe failed");
            exit(EXIT_FAILURE);
        }
        struct timespec start, end;
        timespec_get(&start, TIME_UTC);
        pid_t pid = fork();
        if (pid == 0) {
            // child process
            close(pipefd[0]);
            alarm_setup();
            StatusInfo tstatus = {
                .status = Success,
                .fail_msg = {0},
            };
            alarm(20);
            tv->tests[idx].func(&tstatus);
            int wret = write(pipefd[1], tstatus.fail_msg, 128);
            if (wret == -1) {
                perror("Failed to write to pipe");
            }
            close(pipefd[1]);
            exit(tstatus.status);
        } else {
            close(pipefd[1]);
            // parent process
            int status;
            waitpid(pid, &status, 0);
            timespec_get(&end, TIME_UTC);
            long elapsed_ms =
                (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
            char buf[128] = {0};
            int rret = read(pipefd[0], buf, 128);
            if (rret == -1) {
                perror("Failed to read to pipe");
            }
            close(pipefd[0]);
            if (WIFEXITED(status)) {
                switch ((enum Status)WEXITSTATUS(status)) {
                case Success:
                    string_append(output, " " GREEN "[PASSED, %ldms]" RESET "\n", elapsed_ms);
                    atomic_fetch_add_explicit(tt->passed, 1, memory_order_relaxed);
                    break;
                case Fail:
                    string_append(output, " " RED "[FAILED, %ldms]" RESET "\n\t  > Details: %s\n\n",
                                  elapsed_ms, buf);
                    atomic_fetch_add_explicit(tt->failed, 1, memory_order_relaxed);
                    break;
                case Skip:
                    string_append(output, " " YELLOW "[SKIPPED, ⏭ ]" RESET "\n");
                    atomic_fetch_add_explicit(tt->skipped, 1, memory_order_relaxed);
                    break;
                case Timeout:
                    string_append(output, " " GREY "[TIMEOUT, ⧖ ]" RESET "\n");
                    atomic_fetch_add_explicit(tt->timeout, 1, memory_order_relaxed);
                    break;
                default:
                    unreachable();
                };
            } else if (WIFSIGNALED(status)) {
                string_append(output, " " MAGENTA "[CRASHED, ☠ ]" RESET "\n");
                atomic_fetch_add_explicit(tt->crashed, 1, memory_order_relaxed);
            }
            fprintf(stderr, "%s", output->buf);
            fflush(stderr);
            string_rewind(output);
        }
    }
    string_free(output);
    return 0;
}

// number of threads from env var
static int NUM_THREADS = 1;
void run_all_tests_threaded() {
    // Setup Printing End Column
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        w.ws_col = 80;
    }
    int cols = w.ws_col;
    int max_cols = largest_test_name > cols - 31 ? cols - 31 : largest_test_name;
    int max_suite_cols = largest_suite_name > cols - 31 ? cols - 31 : largest_suite_name;

    String *output = string_init();

    assert(test_suites);
    // Result Header
    khint_t scount = kh_size(test_suites);
    time_t t = time(NULL);
    struct tm timeinfo;
    struct tm *tm = gmtime_r(&t, &timeinfo);
    char date[11];
    char time[9];
    strftime(date, sizeof(date), "%Y-%m-%d", tm);
    strftime(time, sizeof(time), "%H:%M:%S", tm);

    string_append(output, "=== Test Run Started ===\n");
    string_append(output, "Date: %s | Time: %s UTC\n", date, time);
    string_append(output, "%s\n\n", DASHES);
    string_append(output, "Running %zu tests in %d suites\n", tcount, scount);
    string_append(output, "%s\n", DASHES);

    atomic_int passed = 0;
    atomic_int failed = 0;
    atomic_int crashed = 0;
    atomic_int skipped = 0;
    atomic_int timeout = 0;
    khiter_t k;
    for (k = kh_begin(test_suites); k != kh_end(test_suites); ++k) {
        if (kh_exist(test_suites, k)) {
            const char *suite_name = kh_key(test_suites, k);
            TestsVec *tv = kh_val(test_suites, k);
            string_append(output, "\n⣿ Suite: %.*s ⣿\n", max_suite_cols, suite_name);
            printf("%s", output->buf);
            fflush(stdout);
            string_rewind(output);
            // tests per thread
            int tpt = tv->len / NUM_THREADS;
            int rem = tv->len % NUM_THREADS;
            // each thread gets its range from the thread_id * tpt to (thread_id + 1) * tpt
            // the last thread gets the remainder as a bonus
            thrd_t threads[NUM_THREADS];
            TestThread tt[NUM_THREADS];
            for (int i = 0; i < NUM_THREADS; ++i) {
                tt[i] = (TestThread){
                    .tv = tv,
                    .start = i * tpt,
                    .end = (i + 1) * tpt,
                    .passed = &passed,
                    .failed = &failed,
                    .crashed = &crashed,
                    .skipped = &skipped,
                    .timeout = &timeout,
                    .max_cols = max_cols,
                };
                if (i == NUM_THREADS - 1) {
                    tt[i].end += rem;
                }
                thrd_create(&threads[i], thread_func, &tt[i]);
            }
            for (int i = 0; i < NUM_THREADS; ++i) {
                thrd_join(threads[i], NULL);
            }

            test_vec_free(tv);
        }
    }

    kh_destroy(str_map, test_suites);
    string_append(output, "\n%s\n", DASHES);
    string_append(output, "=== Test Run Summary ===\n");
    string_append(output,
                  "Total Tests: %zu | " GREEN "Passed" RESET ": %d | " RED "Failed" RESET
                  ": %d | " MAGENTA "Crashed" RESET ": %d | " YELLOW "Skipped" RESET ": %d | " GREY
                  "Timeout" RESET ": %d\n",
                  tcount, passed, failed, crashed, skipped, timeout);
    string_append(output, "%s\n", DASHES);
    fprintf(stderr, "%s", output->buf);
    string_free(output);
}

__attribute__((weak)) int main(void) {
    NUM_THREADS = getenv("SOUFFLE_THREADS") ? atoi(getenv("SOUFFLE_THREADS")) : 1;
    if (NUM_THREADS > 1) {
        run_all_tests_threaded();
    } else {
        run_all_tests();
    }
    return 0;
}
