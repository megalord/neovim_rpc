#include "cmp.h"

cmp_ctx_t msgpack_socket_init (void * buf);
void print_cmp_obj (cmp_object_t obj, int fh);
int make_named_socket (const char *filename, const bool server);
void error_and_exit(const char *msg);
