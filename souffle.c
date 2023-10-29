#include "souffle.h"
#include "stdlib.h"
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>

void status_print(enum Status status, const char *file, int lineno,
                  const char *fmt, ...) {

  // Initialize the variable argument list
  va_list args;
  va_start(args, fmt);
  const char* ret_char;
  const char* ret_str;
  switch (status) {
    case Success:
      ret_char = "✔";
      ret_str = "OK";
      break;
    case Fail:
      ret_char = "✗";
      ret_str = "FAIL";
      break;
    case Skip:
      ret_char = "↷";
      ret_str = "SKIP";
      break;
    default:
      unreachable();
  };
  // Print the log header

  fprintf(stderr,"\t... %s %s  [0ms] [%s:%d]\n",ret_char, ret_str, file, lineno);
  if (status == Fail) {
    // Print the log message
    fprintf(stderr,"\t  ↳ Details: ");
    vfprintf(stderr, fmt, args);
  }
  fprintf(stderr,"\n\n");

  // Cleanup
  va_end(args);
}

typedef struct TestSuite {
  TestNode *test_list;
  const char* sname;
} TestSuite;

static TestSuite* test_suite;
static TestNode *test_list = NULL;
static size_t tcount = 0;

void register_test(const char *suite, const char *name, TestFunc func) {
  TestNode *new_node = (TestNode *)malloc(sizeof(TestNode));
  assert(new_node && "failed to allocate node");
  new_node->name = name;
  new_node->func = func;
  new_node->next = test_list;
  test_list = new_node;
  tcount += 1;
}

void run_all_tests() {
  printf("=== Test Run Started ===\n");
  printf("Date: 2023-10-27 | Time: 14:30:00\n");
  printf("--------------------------------------------------------\n\n");
  printf("Running %zu tests in 3 suites\n", tcount);
  printf("--------------------------------------------------------\n\n");
  int passed = 0;
  int failed = 0;
  int crashed = 0;
  int skipped = 0;
  TestNode *current = test_list;
  while (current) {
    pid_t pid = fork();
    if (pid == 0) {
      printf("\t• %s  ", current->name);
      fflush(stdout);
      // child process
      enum Status tstatus = Success;
      current->func(&tstatus);
      exit(tstatus);
    } else {
      // parent process
      int status;
      waitpid(pid, &status, 0);

      if (WIFEXITED(status)) {
        switch ((enum Status)WEXITSTATUS(status)) {
          case Success:
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
        printf("\t... ☠ CRASH  [0ms]\n\n");
        crashed += 1;
      }
    }
    // free the node
    TestNode *next = current->next;
    free(current);
    current = next;
  }

  printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  printf("\n\n▣ Summary: %d Passed; %d Failed; %d Crashed; %d Skipped\n",
     passed, failed, crashed, skipped);
}

__attribute__((weak)) int main(void) {
  run_all_tests();
  return 0;
}
