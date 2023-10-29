#include "souffle.h"
#include "stdlib.h"
#include <assert.h>
#include <stdarg.h>

#include "klib/khash.h"
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define MAGENTA "\033[35m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"


KHASH_MAP_INIT_STR(str_map, TestsVec*)
khash_t(str_map) * test_suites;

static size_t tcount = 0;
static int largest_test_name = 0;



TestsVec* test_vec_init() {
  TestsVec* tv = malloc(sizeof(TestsVec));
  assert(tv);
  tv->tests = malloc(128 * sizeof(Test));
  tv->capacity = 128;
  tv->len = 0;
  return tv;
}


void test_vec_free(TestsVec* tv) {
  free(tv->tests);
  free(tv);
}

static void test_vec_push(TestsVec* tv, Test t) {
  if (tv->len == tv->capacity) {
    tv->tests = realloc(tv->tests, (size_t)(tv->capacity * 2) * sizeof(Test));
    assert(tv->tests);
    tv->capacity *= 2;
  }
  tv->tests[tv->len] = t;
  tv->len++;
  return;
}

void err_print(const char *file, int lineno,
                  const char *fmt, ...) {

  // Initialize the variable argument list
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, " " RED "[FAILED, 0ms]" RESET " [%s:%d]\n", file,
          lineno);
  // Print the log message
  fprintf(stderr, "\t  > Details: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n\n");

  // Cleanup
  va_end(args);
}


// TODO: Change the TestNode from a stupid linked list to a vector.
void register_test(const char *suite, const char *name, TestFunc func) {
  if (test_suites == NULL) {
    test_suites = kh_init(str_map);
  }
  int ret;
  khiter_t k = kh_put(str_map, test_suites, suite, &ret);
  if (ret < 0) {
    perror("Failed to access hash table");
    exit(EXIT_FAILURE);
  }
  int test_name_len = strlen(name);
  if (test_name_len > largest_test_name) {
    largest_test_name = test_name_len;
  }
  // in order linked list
  Test t = {
    .func = func,
    .name = name,
  };
  if (ret) { // new key; initialize value to NULL
    kh_value(test_suites, k) = test_vec_init();
  }
  TestsVec *tv = kh_value(test_suites, k);
  test_vec_push(tv, t);
  tcount += 1;
}

void run_all_tests() {
  assert(test_suites);
  khint_t scount = kh_size(test_suites);
  fprintf(stderr, "=== Test Run Started ===\n");
  fprintf(stderr, "Date: 2023-10-27 | Time: 14:30:00\n");
  fprintf(stderr,
          "--------------------------------------------------------\n\n");
  fprintf(stderr, "Running %zu tests in %d suites\n", tcount, scount);
  fprintf(stderr, "--------------------------------------------------------\n");

  int passed = 0;
  int failed = 0;
  int crashed = 0;
  int skipped = 0;
  khiter_t k;
  for (k = kh_begin(test_suites); k != kh_end(test_suites); ++k) {
    if (kh_exist(test_suites, k)) {
      const char *suite_name = kh_key(test_suites, k);
      TestsVec *tv = kh_val(test_suites, k);
      fprintf(stderr, "\n⣿ Suite: %s ⣿\n", suite_name);
      for (size_t idx = 0; idx < tv->len; ++idx){
        pid_t pid = fork();
        if (pid == 0) {
          int padding = largest_test_name - strlen(tv->tests[idx].name);
          fprintf(stderr, "  🧪 %s .........", tv->tests[idx].name);
          for (int i = 0; i < padding; ++i) {
            fprintf(stderr, ".");
          }
          // child process
          enum Status tstatus = Success;
          tv->tests[idx].func(&tstatus);
          exit(tstatus);
        } else {
          // parent process
          int status;
          waitpid(pid, &status, 0);

          if (WIFEXITED(status)) {
            switch ((enum Status)WEXITSTATUS(status)) {
            case Success:
              fprintf(stderr, " " GREEN "[PASSED, 0ms]" RESET "\n");
              passed += 1;
              break;
            case Fail:
              failed += 1;
              break;
            case Skip:
              fprintf(stderr, " " YELLOW "[SKIPPED, 0ms]" RESET "\n");
              skipped += 1;
              break;
            default:
              assert(false && "Unreachable");
              // unreachable();        // TODO: Uncomment me on CLANG 17
            };
          } else if (WIFSIGNALED(status)) {
              fprintf(stderr, " " MAGENTA "[CRASHED, ☠ ]" RESET "\n");
              crashed += 1;
          }
        }
      }
      test_vec_free(tv);
    }
  }

  kh_destroy(str_map, test_suites);
  fprintf(stderr,
          "\n--------------------------------------------------------\n");
  fprintf(stderr, "=== Test Run Summary ===\n");
  fprintf(stderr,
          "Total Tests: %zu | " GREEN "Passed" RESET ": %d | " RED
          "Failed" RESET ": %d | " MAGENTA "Crashed" RESET ": %d | " YELLOW
          "Skipped" RESET ": "
          "%d\n",
          tcount, passed, failed, crashed, skipped);
  fprintf(stderr, "--------------------------------------------------------\n");
}

__attribute__((weak)) int main(void) {
  run_all_tests();
  return 0;
}
