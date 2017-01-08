#include <stdio.h>

#include "rpc.h"
#include "rpc_methods.h"
#include "socket.h"

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

  for (int i = 0; i < num_bufs; i++) {
    printf("%02d\n", ((char *) bufs)[i]);
  }

  free(bufs);
  rpc_end();
  return 0;
}
