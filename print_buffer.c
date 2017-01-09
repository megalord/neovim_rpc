#include <stdio.h>

#include "rpc.h"
#include "rpc_methods.h"

int main (int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Supply a file name");
    return 1;
  }
  rpc_init_socket(argv[1]);

  uint32_t num_bufs;
  Buffer *bufs;
  if (!nvim_list_bufs(&bufs, &num_bufs)) {
    rpc_end();
    return 1;
  }

  if (num_bufs == 0) {
    printf("No buffers open!\n");
    rpc_end();
    return 1;
  }

  printf("Current buffers:\n");
  for (int i = 0; i < num_bufs; i++) {
    printf("%02i\n", bufs[i]);
  }

  int buf_i;
  printf("Select buffer: ");
  int c1 = getchar() - 48;
  if (c1 < 0 || c1 > 9) {
  }
  int c2 = getchar() - 48;
  if (c2 < 0 || c2 > 9) {
  }
  int buf_num = c1 * 10 + c2;

  int i = 0;
  while (i < num_bufs && buf_num != bufs[i]) {
    i++;
  }
  if (i == num_bufs) {
    printf("Buffer %02i does not exist.\n", buf_num);
    rpc_end();
    return 1;
  }

  Buffer b = bufs[i];
  free(bufs);

  char *name;
  if (!nvim_buf_get_name(b, &name)) {
    rpc_end();
    return 1;
  }

  printf("file: %s\n", name);
  free(name);

  uint32_t num_lines;
  char **lines;
  if (!nvim_buf_get_lines(b, 0, -1, false, &lines, &num_lines)) {
    rpc_end();
    return 1;
  }

  printf("%i lines\n", num_lines);
  for (int i = 0; i < num_lines; i++) {
    printf("%03i: %s\n", i, lines[i]);
    free(lines[i]);
  }
  free(lines);

  rpc_end();
  return 0;
}
