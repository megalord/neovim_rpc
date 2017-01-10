// needed for cmp.h
#include <stdint.h>
#include <stdbool.h>

#include "cmp.h"

typedef enum {
  NVIM_RPC_REQUEST,
  NVIM_RPC_RESPONSE,
  NVIM_RPC_NOTIFY
} rpc_type;

//typedef struct {
//  uint8_t size;
//  //rpc_content_type type;
//  void *data;
//} rpc_content;

typedef struct {
  uint8_t id;
  rpc_type type;
  char *title;
  //content_type;
  uint32_t size;
  void *data;
} rpc_message;


typedef enum {
  STDIN_STDOUT, // for use with nvim's jobstart(_, {rpc: v:true})
  TCP_SOCKET, // NVIM_LISTEN_ADDRESS
  NAMED_SOCKET, // echo v:servername
  EMBEDDED
} rpc_connection_method;

typedef union {
  char *filename;
  FILE *fh;
} rpc_connection_address;

typedef struct {
  rpc_connection_method method;
  rpc_connection_address address;
} rpc_connection;

cmp_ctx_t cmp;

void rpc_init_stdio (void);
void rpc_init_socket (char name[]);
bool rpc_send (rpc_type t, char method[], int num_args);
void rpc_end (void);
bool wait_for_response(rpc_message *msg);
bool read_message (rpc_message *msg);
bool read_message_headers (void);
bool read_string (char **str);
