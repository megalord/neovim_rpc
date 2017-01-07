#include <stdio.h>

// needed for cmp.h
#include <stdint.h>
#include <stdbool.h>

#include "cmp.h"
#include "rpc.h"
#include "rpc_methods.h"
#include "socket.h"

int main (int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Supply a file name");
    return 1;
  }
  rpc_init_socket(argv[1]);

  int a = 0;
  if (!nvim_list_bufs(&a)) {
    rpc_end();
    return 1;
  }

  cmp_object_t cmp_obj;
  while (cmp_read_object(&cmp, &cmp_obj)) {
    print_cmp_obj(cmp_obj, *(int *) cmp.buf);
  }

  rpc_end();
  return 0;
}
