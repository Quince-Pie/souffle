#ifndef _WIN32
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <threads.h>
#include <unistd.h>
#else
#include <windows.h>
#define TRY __try
#define EXCEPT __except (EXCEPTION_EXECUTE_HANDLER)
#endif // _WIN32

#include <stdarg.h>
#include <stdatomic.h>
#include <stddef.h>
#include <time.h>
#include "klib/khash.h"
#include "souffle.h"
#include "stdlib.h"

#ifndef SOUFFLE_NOCOLOR
#define UNDERLINED "\033[4m"
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define MAGENTA "\033[35m"
#define YELLOW "\033[0;33m"
#define GREY "\033[90m"
#define RESET "\033[0m"
#else
#define UNDERLINED ""
#define GREEN ""
#define RED ""
#define MAGENTA ""
#define YELLOW ""
#define GREY ""
#define RESET ""
#endif

// Separator
static const char *DASHES =
    "_________________________________________________________________________________";

static SouffleString *string_init() {
    SouffleString *str = malloc(sizeof(SouffleString));
    assert(str);
    str->buf = malloc(1024 * sizeof(char));
    assert(str->buf);
    str->capacity = 1024;
    str->len = 0;
    return str;
}

static void string_free(SouffleString *str) {
    free(str->buf);
    free(str);
}
static inline void string_rewind(SouffleString *str) {
    str->len = 0;
    str->buf[0] = '\0';
}
// dump a string to stdout and rewind
static inline void string_dump(SouffleString *str) {
    fprintf(stdout, "%s", str->buf);
    fflush(stdout);
    string_rewind(str);
}

void string_append(SouffleString *str, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size_needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    va_start(args, fmt);
    if (size_needed + 1 > str->capacity) {
        string_dump(str);
        free(str->buf);
        str->capacity *= 2;
        str->buf = malloc(str->capacity * sizeof(char));
        assert(str->buf);
    } else if (size_needed + 1 > str->capacity - str->len) {
        string_dump(str);
    }
    str->len += vsnprintf(str->buf + str->len, str->capacity, fmt, args);
    // Cleanup
    va_end(args);
}

static void string_append_va(SouffleString *str, const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    size_t size_needed = vsnprintf(NULL, 0, fmt, args);
    if (size_needed + 1 > str->capacity) {
        string_dump(str);
        free(str->buf);
        str->capacity *= 2;
        str->buf = malloc(str->capacity * sizeof(char));
        assert(str->buf);
    } else if (size_needed + 1 > str->capacity - str->len) {
        string_dump(str);
    }
    str->len += vsnprintf(str->buf + str->len, str->capacity, fmt, args_copy);
    va_end(args_copy);
}

void souffle_log_msg(StatusInfo *status_info, const char *file, int lineno, const char *fmt, ...) {
    if (status_info->msg == NULL) {
        status_info->msg = string_init();
    }
    string_append(status_info->msg, "\t  > [" UNDERLINED "%s:%d" RESET "]:", file, lineno);
    string_append(status_info->msg, "\n\t  >> ");
    va_list args;
    va_start(args, fmt);
    string_append_va(status_info->msg, fmt, args);
    va_end(args);
}

void souffle_log_msg_raw(StatusInfo *status_info, const char *fmt, ...) {
    if (status_info->msg == NULL) {
        status_info->msg = string_init();
    }
    va_list args;
    va_start(args, fmt);
    string_append_va(status_info->msg, fmt, args);
    va_end(args);
}

KHASH_MAP_INIT_STR(str_map, TestsVec *)
khash_t(str_map) * test_suites;

static size_t tcount = 0;
static int largest_name = 0;

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

void register_test(const char *suite, const char *name, TestFunc func, SetupFunc setup,
                   TeardownFunc teardown) {
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
    if (test_name_len > largest_name) {
        largest_name = test_name_len;
    }
    int suite_name_len = strlen(suite);
    if (suite_name_len > largest_name) {
        largest_name = suite_name_len;
    }
    Test t = {
        .func = func,
        .name = name,
        .setup = setup,
        .teardown = teardown,
    };
    if (ret > 0) { // new key; initialize value to a new vec
        kh_value(test_suites, k) = test_vec_init();
    }
    TestsVec *tv = kh_value(test_suites, k);
    test_vec_push(tv, t);
    tcount += 1;
}

#ifndef _WIN32
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

int run_all_tests() {
    const char *timeout_str = getenv("SOUFFLE_TIMEOUT");
    volatile int timeout_time = timeout_str ? atoi(timeout_str) : 20;
    if (timeout_time == 0) {
        timeout_time = 20;
    }
    // Setup Printing End Column
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        w.ws_col = 80;
    }
    int cols = w.ws_col;
    int max_cols = cols > 53 ? 53 : cols;
    max_cols =
        largest_name < max_cols ? max_cols : (largest_name > cols - 4 ? cols - 4 : largest_name);

    SouffleString *output = string_init();

    assert(test_suites);
    khint_t scount = kh_size(test_suites);

    // Result Header
    string_append(output, "=== Test Run Started ===\n");
    string_append(output, "%.*s\n\n", max_cols, DASHES);
    string_append(output, "Running %zu tests in %d suites\n", tcount, scount);
    string_append(output, "%.*s\n\n", max_cols, DASHES);

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
            int spaces_required = max_cols - 11 - strlen(suite_name);
            if (spaces_required < 0)
                spaces_required = 0;
            string_append(output, "⣿ Suite: %.*s %*s⣿\n", max_cols - 11, suite_name,
                          spaces_required, "");
            for (volatile size_t idx = 0; idx < tv->len; ++idx) {
                int padding = max_cols - strlen(tv->tests[idx].name) - 28;
                string_append(output, "  %s 🧪 %.*s ......", tv->tests[idx].setup ? "⚙" : " ",
                              max_cols - 28, tv->tests[idx].name);

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
                // use vfork to avoid copying memory
                pid_t pid = vfork();
                if (pid == 0) {
                    // child process
                    close(pipefd[0]);
                    alarm_setup();
                    StatusInfo tstatus = {
                        .status = Success,
                        .msg = NULL,
                    };
                    alarm(timeout_time);
                    void *ctx_internl = NULL;
                    void **ctx = &ctx_internl;
                    if (tv->tests[idx].setup) {
                        tv->tests[idx].setup(ctx);
                    }
                    tv->tests[idx].func(&tstatus, ctx);
                    if (tv->tests[idx].teardown) {
                        tv->tests[idx].teardown(ctx);
                    }
                    if (tstatus.msg) {
                        int wret = write(pipefd[1], &tstatus.msg->len, sizeof(int));
                        wret |= write(pipefd[1], tstatus.msg->buf, tstatus.msg->len);
                        if (wret == -1) {
                            perror("Failed to write to pipe");
                        }
                        string_free(tstatus.msg);
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
                    int buf_size = 0;
                    int rret = read(pipefd[0], &buf_size, sizeof(int));
                    // calloc for the null terminator
                    char *err_buf = buf_size ? calloc((buf_size + 1), sizeof(char)) : NULL;
                    rret = read(pipefd[0], err_buf, buf_size);
                    if (rret == -1) {
                        perror("Failed to read to pipe");
                    }
                    if (WIFEXITED(status)) {
                        switch ((enum Status)WEXITSTATUS(status)) {
                        case Success:
                            string_append(output, " " GREEN "[PASSED, %ldms]" RESET "\n%s\n",
                                          elapsed_ms, err_buf ? err_buf : "");
                            passed += 1;
                            break;
                        case Fail: {
                            string_append(output, " " RED "[FAILED, %ldms]" RESET "\n%s\n",
                                          elapsed_ms, err_buf ? err_buf : "");
                            failed += 1;
                            break;
                        }
                        case Skip:
                            string_append(output, " " YELLOW "[SKIPPED, ⏭ ]" RESET "\n%s\n",
                                          err_buf ? err_buf : "");
                            skipped += 1;
                            break;
                        case Timeout:
                            string_append(output, " " GREY "[TIMEOUT, ⧖ ]" RESET "\n%s\n",
                                          err_buf ? err_buf : "");
                            timeout += 1;
                            break;
                        default:
                            unreachable();
                        };
                    } else if (WIFSIGNALED(status)) {
                        string_append(output, " " MAGENTA "[CRASHED, ☠ ]" RESET "\n\n");
                        crashed += 1;
                    }
                    free(err_buf);
                    close(pipefd[0]);
                }
            }
            test_vec_free(tv);
        }
    }

    kh_destroy(str_map, test_suites);
    string_append(output, "%.*s\n\n", max_cols, DASHES);
    string_append(output, "=== Test Run Summary ===\n");
    string_append(output,
                  "Total Tests: %zu | " GREEN "Passed" RESET ": %d | " RED "Failed" RESET
                  ": %d | " MAGENTA "Crashed" RESET ": %d | " YELLOW "Skipped" RESET ": %d | " GREY
                  "Timeout" RESET ": %d\n",
                  tcount, passed, failed, crashed, skipped, timeout);
    string_append(output, "%.*s\n", max_cols, DASHES);
    fprintf(stdout, "%s", output->buf);
    string_free(output);
    if (crashed > 0 || failed > 0 || timeout > 0) {
        return 1;
    }
    return 0;
}
#else
// Windows

typedef struct ThreadInfo {
    Test *test;
    StatusInfo *status_info;
} ThreadInfo;

DWORD WINAPI func_exec_timeout_win(LPVOID lpParam) {
    ThreadInfo *ti = (ThreadInfo *)lpParam;
    void *ctx_internl = NULL;
    void **ctx = &ctx_internl;
    TRY {
        if (ti->test->setup) {
            ti->test->setup(ctx);
        }
        if (ti->test->func) {
            ti->test->func(ti->status_info, ctx);
        }
        if (ti->test->teardown) {
            ti->test->teardown(ctx);
        }
        return 0;
    }
    EXCEPT {
        ti->status_info->status = Crashed;
        return 0;
    }
}

int run_all_tests_win() {
    const char *var_name = "SOUFFLE_TIMEOUT";
    size_t required_size;
    DWORD timeout_time = 20000;
    getenv_s(&required_size, NULL, 0, var_name);
    if (required_size != 0) {
        char env_value[required_size];
        getenv_s(&required_size, env_value, required_size, var_name);
        errno = 0;
        char *endptr;
        unsigned long ret = strtoul(env_value, &endptr, 10);
        if (endptr != env_value && errno == 0) {
            timeout_time = ret * 1000;
        }
    }
    // Setup Printing End Column. Since we don't have ioctl in windows we need to use windows.h
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int max_cols = cols > 53 ? 53 : cols;
    max_cols =
        largest_name < max_cols ? max_cols : (largest_name > cols - 4 ? cols - 4 : largest_name);

    SouffleString *output = string_init();

    assert(test_suites);
    // Result Header
    khint_t scount = kh_size(test_suites);

    string_append(output, "=== Test Run Started ===\n");
    string_append(output, "%.*s\n\n", max_cols, DASHES);
    string_append(output, "Running %zu tests in %d suites\n", tcount, scount);
    string_append(output, "%.*s\n\n", max_cols, DASHES);

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
            int spaces_required = max_cols - 11 - strlen(suite_name);
            if (spaces_required < 0)
                spaces_required = 0;
            string_append(output, "⣿ Suite: %.*s %*s⣿\n", max_cols - 11, suite_name,
                          spaces_required, "");
            for (size_t idx = 0; idx < tv->len; ++idx) {

                int padding = max_cols - strlen(tv->tests[idx].name) - 28;
                string_append(output, "  %s 🧪 %.*s ......", tv->tests[idx].setup ? "⚙" : " ",
                              max_cols - 28, tv->tests[idx].name);
                for (int i = 0; i < padding; ++i) {
                    string_append(output, ".");
                }
                struct timespec start, end;
                timespec_get(&start, TIME_UTC);
                StatusInfo tstatus = {
                    .status = Success,
                    .msg = NULL,
                };

                // spawn a thread to run the test
                ThreadInfo tinfo = {
                    .test = &tv->tests[idx],
                    .status_info = &tstatus,
                };
                HANDLE thread = CreateThread(NULL, 0, func_exec_timeout_win, &tinfo, 0, NULL);
                if (thread == NULL) {
                    perror("Failed to create thread");
                    exit(EXIT_FAILURE);
                }
                // wait for the thread to finish with a timeout
                DWORD wait_result = WaitForSingleObject(thread, timeout_time);
                if (wait_result == WAIT_TIMEOUT) {
                    tstatus.status = Timeout;
                } else if (wait_result == WAIT_FAILED) {
                    tstatus.status = Crashed;
                }
                timespec_get(&end, TIME_UTC);
                long elapsed_ms =
                    (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
                char *err_buf = NULL;
                if (tstatus.msg) {
                    err_buf = tstatus.msg->buf;
                }
                switch (tstatus.status) {
                case Success:
                    string_append(output, " " GREEN "[PASSED, %ldms]" RESET "\n%s\n", elapsed_ms,
                                  err_buf ? err_buf : "");
                    passed += 1;
                    break;
                case Fail:
                    string_append(output, " " RED "[FAILED, %ldms]" RESET "\n%s\n", elapsed_ms,
                                  err_buf ? err_buf : "");
                    failed += 1;

                    break;
                case Skip:
                    string_append(output, " " YELLOW "[SKIPPED, ⏭ ]" RESET "\n%s\n",
                                  err_buf ? err_buf : "");
                    skipped += 1;
                    break;
                case Timeout:
                    string_append(output, " " GREY "[TIMEOUT, ⧖ ]" RESET "\n%s\n",
                                  err_buf ? err_buf : "");
                    timeout += 1;
                    break;
                case Crashed:
                    string_append(output, " " MAGENTA "[CRASHED, ☠ ]" RESET "\n\n");
                    crashed += 1;
                    break;
                default:
                    assert(0 && "Unreachable");
                };
                if (tstatus.msg) {
                    string_free(tstatus.msg);
                }
                CloseHandle(thread);
            }
            test_vec_free(tv);
        }
    }

    kh_destroy(str_map, test_suites);
    string_append(output, "%.*s\n\n", max_cols, DASHES);
    string_append(output, "=== Test Run Summary ===\n");
    string_append(output,
                  "Total Tests: %zu | " GREEN "Passed" RESET ": %d | " RED "Failed" RESET
                  ": %d | " MAGENTA "Crashed" RESET ": %d | " YELLOW "Skipped" RESET ": %d | " GREY
                  "Timeout" RESET ": %d\n",
                  tcount, passed, failed, crashed, skipped, timeout);
    string_append(output, "%.*s\n", max_cols, DASHES);
    fprintf(stdout, "%s", output->buf);
    string_free(output);
    if (crashed > 0 || failed > 0 || timeout > 0) {
        return 1;
    }
    return 0;
}
#endif

__attribute__((weak)) int main(void) {
#ifndef _WIN32
    int ret = run_all_tests();
#else
    int ret = run_all_tests_win();
#endif
    return ret;
}
