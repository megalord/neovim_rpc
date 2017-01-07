#include <stdio.h>

#include "cmp.h"
#include "rpc.h"
#include "socket.h"
#include "test.h"

int main (void) {
  rpc_init_socket("/var/folders/m6/fb2myxz50tg3jhrfdyz6v3n8hssc_l/T/nvimeyoFKs/0");
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
