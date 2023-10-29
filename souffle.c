#include "souffle.h"
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

  const char *status_str;
  switch (status) {
  case Success:
    status_str = "[SUCCESS]";
    break;
  case Fail:
    status_str = "[FAILURE]";
    break;
  case Skip:
    status_str = "[SKIPPED]";
    break;
  default:
    unreachable();
  };
  // Print the log header

  fprintf(stderr, "%s\n", status_str);
  if (status == Fail) {
    // Print the log message
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "   [%s:%d]\n", file, lineno);
  }

  // Cleanup
  va_end(args);
}
static TestNode *test_list = NULL;

void register_test(const char *suite, const char *name, TestFunc func) {
  TestNode *new_node = (TestNode *)malloc(sizeof(TestNode));
  assert(new_node && "failed to allocate node");
  new_node->name = name;
  new_node->func = func;
  new_node->next = test_list;
  test_list = new_node;
}

void run_all_tests() {
  TestNode *current = test_list;
  while (current) {
    pid_t pid = fork();
    if (pid == 0) {
      // child process
      current->func();
      exit(0);
    } else {
      // parent process
      int status;
      waitpid(pid, &status, 0);
      if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Test %s exited with error code %d\n", current->name,
                WEXITSTATUS(status));
      }
      if (WIFSIGNALED(status)) {
        fprintf(stderr, "Test %s crashed with signal %d!\n", current->name,
                WTERMSIG(status));
      }
    }
    // free the node
    TestNode *next = current->next;
    free(current);
    current = next;
  }
}

__attribute__((weak)) int main(void) {
  run_all_tests();
  return 0;
}
