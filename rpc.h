#ifndef RPC_H_
#define RPC_H_

#include <stdbool.h>
#include "cmp.h"

typedef enum {
  NVIM_RPC_REQUEST,
  NVIM_RPC_RESPONSE,
  NVIM_RPC_NOTIFY
} rpc_type;

cmp_ctx_t cmp;

void rpc_init_stdio (void);
void rpc_init_socket (char name[]);
bool rpc_send (rpc_type t, char method[], int num_args);
void rpc_end (void);

#endif
