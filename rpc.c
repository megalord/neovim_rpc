#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// needed for cmp.h
#include <stdint.h>
#include <stdbool.h>

#include "cmp.h"
#include "rpc.h"
#include "socket.h"

enum rpc_connection_method {
  STDIN_STDOUT, // for use with nvim's jobstart(_, {rpc: v:true})
  TCP_SOCKET, // NVIM_LISTEN_ADDRESS
  NAMED_SOCKET, // echo v:servername
  EMBEDDED
};

bool stdin_reader (cmp_ctx_t *cmp, void *data, size_t limit) {
  return fread(data, 1, limit, stdin) == limit;
}

size_t stdout_writer (cmp_ctx_t *cmp, const void *data, size_t count) {
  return fwrite(data, 1, count, stdout);
}

void rpc_init_stdio (void) {
  cmp_init(&cmp, NULL, stdin_reader, stdout_writer); // cmp->buf should never be used
}

void rpc_init_socket (char name[]) {
  int *sock = malloc(sizeof(int));
  *sock = make_named_socket(name, false);
  cmp = msgpack_socket_init(sock);
}

void rpc_end () {
  close(*(int *) cmp.buf);
  free(cmp.buf);
}

bool rpc_send (rpc_type t, char method[], int num_args) {
  //if (cmp != NULL) {
  //  return false;
  //}

  if (!cmp_write_array(&cmp, 4)) {
    //cmp_strerror(&cmp);
    return false;
  }
  if (!cmp_write_uint(&cmp, 0)) {
    return false;
  }
  if (!cmp_write_uint(&cmp, 0)) {
    return false;
  }
  if (!cmp_write_str(&cmp, method, strlen(method))) {
    return false;
  }
  if (!cmp_write_array(&cmp, num_args)) {
    return false;
  }
  return true;
}
