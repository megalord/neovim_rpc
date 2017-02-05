#ifndef TEST_H_
#define TEST_H_

#include <stdio.h>
#include <wchar.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define pass() indicate_pass(); return;
#define fail(str) indicate_fail(str, __FILE__, __LINE__); return;
#define fail2(a, b, template) indicate_fail("", __FILE__, __LINE__); printf(template "\n", a, b); return;

#define assertEqual(expected, actual, template) if (expected == actual) {\
  pass();\
} else {\
  fail2(expected, actual, template);\
}

#define assertEqualCust(equal, expected, actual, template) if (equal) {\
  pass();\
} else {\
  fail2(expected, actual, template);\
}

#ifndef MAX_NESTING
#define MAX_NESTING 3
#endif

uint8_t num_descriptions = 0;
char **descriptions;

void init_test_suite () {
  descriptions = malloc(MAX_NESTING * sizeof(char *));
}

void prefix_push(char *str) {
  if (num_descriptions == MAX_NESTING) {
    fprintf(stderr, "attempting to exceed nesting level of %u for test %s, aborting\n", num_descriptions, str);
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

void print_description () {
  for (int i = 0; i < num_descriptions; i++) {
    printf(" %s", descriptions[i]);
  }
  printf("\n");
}

void indicate_pass () {
  printf(GRN "%lc", L'\u2713');
  print_description(); 
  printf(RESET);
}

void indicate_fail (char *str, char *file, int line) {
  printf(RED "%lc", L'\u2717');
  print_description();
  printf("%s", str);
  printf("\nat %s:%u\n" RESET, file, line);
}

void describe (char *description, void (*fn)(void)) {
  prefix_push(description);
  fn();
  prefix_pop();
}

void it (char *description, void (*fn)(void)) {
  prefix_push(description);
  fn();
  prefix_pop();
}

#endif
