#ifndef TEST_H_
#define TEST_H_

#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <wchar.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

//#define pass() indicate_pass(); return;
//#define fail() indicate_fail(); printf(" "); print_line(__FILE__, __LINE__); return;
#define pass() return PASS;
#define fail() return FAIL;
#define fail_msg(a, b, template) indicate_fail(); printf("  " template, a, b); print_line(__FILE__, __LINE__); return;

#define assert_equal(expected, actual, template) if (expected == actual) {\
  pass();\
} else {\
  fail_msg(expected, actual, template);\
}

#define assert_equal_str(expected, actual) \
if (strcmp(expected, actual) == 0) {\
  pass();\
} else {\
  fail_msg(expected, actual, "%s != %s");\
}

#define assert_true(value) \
if (value) {\
  pass();\
} else {\
  fail();\
}

#ifndef MAX_NESTING
#define MAX_NESTING 3
#endif

typedef enum {
  FAIL,
  PASS
} test_status_t;

typedef struct {
  test_status_t status,
  char *message
} test_result_t;

uint8_t desc_level = 0, num_descriptions = 0, num_passed = 0, num_tests = 0;
char **descriptions;
long start_time;

long get_ms () {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1.0e6 + tv.tv_usec;
}

void init_test_suite () {
  start_time = get_ms();
  descriptions = malloc(MAX_NESTING * sizeof(char *));
}

int end_test_suite () {
  long end_time = get_ms();
  bool all_passed = num_passed == num_tests;
  all_passed ? printf(GRN) : printf(RED);
  printf("\n%u of %u passed" RESET " in %4.3fs\n", num_passed, num_tests, (end_time - start_time) / 1.0e6);
  return all_passed ? 0 : 1;
}

void prefix_push(char *str) {
  if (num_descriptions == MAX_NESTING) {
    fprintf(stderr, "attempting to exceed nesting level of %u for test %s, aborting\n", num_descriptions, str);
    return;
  }
  descriptions[num_descriptions] = malloc(strlen(str));
  strcpy(descriptions[num_descriptions], str);
  num_descriptions++;
}

void prefix_pop() {
  if (num_descriptions == 0) {
    fprintf(stderr, "no prefix to pop\n");
  }
  num_descriptions--;
  free(descriptions[num_descriptions]);
}

void pad (int n) {
  for (int i = 0; i < n; i++) {
    printf(" ");
  }
}

void print_description () {
  for (int i = 0; i < num_descriptions; i++) {
    pad(i * 2);
    printf("%s\n", descriptions[i]);
  }
}

void indicate_pass () {
  num_passed++;
  printf(GRN "%lc", L'\u2713');
  print_description(); 
  printf(RESET);
}

void indicate_fail () {
  printf(RED "%lc", L'\u2717');
  print_description();
}

void print_line (char *file, int line) {
  printf(" at %s:%u\n" RESET, file, line);
}

void describe (char *description, void (*fn)(void)) {
  prefix_push(description);
  fn();
  prefix_pop();
}

void it (char *description, test_result_t (*fn)(void)) {
  prefix_push(description);
  num_tests++;
  fn();
  prefix_pop();
}

#endif
