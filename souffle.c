#include "souffle.h"
#include "stdlib.h"
#include <assert.h>
#include <stdarg.h>

#include "klib/khash.h"
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>

void status_print(enum Status status, const char *file, int lineno,
                  const char *fmt, ...) {

  // Initialize the variable argument list
  va_list args;
  va_start(args, fmt);
  const char *ret_str;
  switch (status) {
  // NOP, pass is handled in the test runner since we want to know that every assertion was correct
  case Success:
    ret_str = "PASSED";
    break;
  case Fail:
    ret_str = "FAILED";
    break;
  case Skip:
    ret_str = "SKIPPED";
    break;
  default:
    unreachable();
  };
  // Print the log header

  if (status == Fail) {
    fprintf(stderr, " [%s, 0ms] [%s:%d]\n", ret_str, file,
            lineno);
    // Print the log message
    fprintf(stderr, "\t  > Details: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n\n");
  }

  // Cleanup
  va_end(args);
}

KHASH_MAP_INIT_STR(str_map, TestNode *)
khash_t(str_map) *test_suites;

static size_t tcount = 0;
static int largest_test_name = 0;
//TODO: Change the TestNode from a stupid linked list to a vector.
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
  TestNode *node = malloc(sizeof(TestNode));
  node->name = name;
  node->func = func;
  node->next = NULL;
  if (ret) { // new key; initialize value to NULL
    kh_value(test_suites, k) = NULL;
  }
  TestNode *head = kh_value(test_suites, k);
  if (!head) {
    kh_value(test_suites, k) = node;
  } else {
    while (head->next) {
      head = head->next;
    }
    head->next = node;
  }
  tcount += 1;
}

void run_all_tests() {
  khint_t scount = kh_size(test_suites);
  printf("=== Test Run Started ===\n");
  printf("Date: 2023-10-27 | Time: 14:30:00\n");
  printf("--------------------------------------------------------\n\n");
  printf("Running %zu tests in %d suites\n", tcount,scount);
  printf("--------------------------------------------------------\n");
  int passed = 0;
  int failed = 0;
  int crashed = 0;
  int skipped = 0;
  khiter_t k;
  for (k = kh_begin(test_suites); k != kh_end(test_suites); ++k) {
    if (kh_exist(test_suites, k)) {
      const char *suite_name = kh_key(test_suites, k);
      TestNode *test = kh_val(test_suites, k);
      printf("\n:: Suite: %s ::\n", suite_name);
      while (test) {
        pid_t pid = fork();
        if (pid == 0) {
          int padding = largest_test_name -  strlen(test->name);
          printf("  🧪 %s .........", test->name);
          for (int i = 0; i < padding; ++i) {
            printf(".");
          }
          fflush(stdout);
          // child process
          enum Status tstatus = Success;
          test->func(&tstatus);
          exit(tstatus);
        } else {
          // parent process
          int status;
          waitpid(pid, &status, 0);

          if (WIFEXITED(status)) {
            switch ((enum Status)WEXITSTATUS(status)) {
            case Success:
              fprintf(stderr, " [PASSED, 0ms]\n");
              passed += 1;
              break;
            case Fail:
              failed += 1;
              break;
            case Skip:
              skipped += 1;
              break;
            default:
              unreachable();
            };
          } else if (WIFSIGNALED(status)) {
            fprintf(stderr, " [CRASHED, ☠ ]\n");
            crashed += 1;
          }
        }
        // free the node
        TestNode *next = test->next;
        free(test);
        test = next;
      }
      // kh_del(str_map, test_suites, k);
    }
  }

  kh_destroy(str_map, test_suites);
  printf("\n--------------------------------------------------------\n");
  printf("=== Test Run Summary ===\n");
  printf("Total Tests: %zu | Passed: %d | Failed: %d | Crashed: %d | Skipped: %d\n", tcount, passed, failed, crashed, skipped);
  // printf("\n\n▣ Summary: %d Passed; %d Failed; %d Crashed; %d Skipped\n",
  //        passed, failed, crashed, skipped);
  printf("--------------------------------------------------------");
}

__attribute__((weak)) int main(void) {
  run_all_tests();
  return 0;
}
