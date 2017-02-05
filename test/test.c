#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvim_rpc.h"
#include "test.h"

void test_set_lines () {
  const uint32_t num_lines = 3;
  char *lines[num_lines] = { "this", "is", "cool" };
  uint32_t num_read_lines;
  char **read_lines;

  if (!nvim_buf_set_lines(0, 0, -1, false, lines, num_lines)) {
    fail("nvim_buf_set_lines failed");
  }

  if (!nvim_buf_get_lines(0, 0, -1, false, &read_lines, &num_read_lines)) {
    fail("nvim_buf_get_lines failed");
  }

  assertEqual(num_lines, num_read_lines, "%u %u");
  for (int i = 0; i < num_lines; i++) {
    assertEqual(lines[i], read_lines[i], "%s %s");
    free(read_lines[i]);
  }
  pass();
}

void fixture_tests () {
  nvim_rpc_connection_address address;
  address.nothing = NULL;
  nvim_rpc_start(EMBEDDED, address);

  it("writes set_lines correctly", test_set_lines);

  nvim_rpc_end();
}

void test_true_false () {
  assertEqual(true, false, "%d is not %d");
}

void test_string_equals () {
  assertEqualCust(strcmp("a", "b") == 0, "a", "b", "%s is not %s");
}

void basic_tests () {
  it("asserts true is not false", test_true_false);
  it("asserts strings", test_string_equals);
}

int main () {
  setlocale(LC_ALL, "");
  init_test_suite();
  describe("basics", basic_tests);
  describe("fixture tests", fixture_tests);
  return 0;
}
