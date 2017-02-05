// needed for cmp.h
#include <stdint.h>
#include <stdbool.h>

#include "cmp.h"

typedef enum {
  STDIN_STDOUT, // for use with nvim's jobstart(_, {rpc: v:true})
  //TCP_SOCKET, // NVIM_LISTEN_ADDRESS
  NAMED_SOCKET, // echo v:servername
  EMBEDDED
} nvim_rpc_connection_method;

typedef union {
  void *nothing;
  char *filename;
} nvim_rpc_connection_address;

void nvim_rpc_start (nvim_rpc_connection_method method, nvim_rpc_connection_address address);
void nvim_rpc_end (void);
const char* nvim_rpc_error (void);
