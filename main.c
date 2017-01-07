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

  rpc_message result;
  if (!nvim_list_bufs(&result)) {
    rpc_end();
    return 1;
  }

  for (int i = 0; i < result.size; i++) {
    printf("%02d\n", ((char *) result.data)[i]);
  }

  rpc_end();
  return 0;
}
