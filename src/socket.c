#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "cmp.h"
#include "rpc.h"

bool read_bytes(void *data, size_t sz, int sock) {
  return read(sock, data, sz) == sz;
}

bool socket_reader(cmp_ctx_t *ctx, void *data, size_t limit) {
  return read_bytes(data, limit, *((int *) ctx->buf));
}

size_t socket_writer(cmp_ctx_t *ctx, const void *data, size_t count) {
  return write(*((int *) ctx->buf), data, count);
}

cmp_ctx_t msgpack_socket_init (void * buf) {
  cmp_ctx_t cmp;
  cmp_init(&cmp, buf, socket_reader, socket_writer);
  return cmp;
}

int make_named_socket (const char *filename, const bool server) {
  struct sockaddr_un name;
  int sock;
  size_t size;

  /* Create the socket. */
  sock = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  /* Bind a name to the socket. */
  name.sun_family = AF_LOCAL;
  strncpy(name.sun_path, filename, sizeof (name.sun_path));
  name.sun_path[sizeof (name.sun_path) - 1] = '\0';

  /* The size of the address is
     the offset of the start of the filename,
     plus its length (not including the terminating null byte).
     Alternatively you can just do:
     size = SUN_LEN (&name);
 */
  size = (offsetof(struct sockaddr_un, sun_path) + strlen(name.sun_path));

  if (server) {
    if (bind(sock, (struct sockaddr *) &name, size) < 0) {
      perror("bind");
      exit(EXIT_FAILURE);
    }
  } else {
    if (connect(sock, (struct sockaddr *) &name, size) < 0) {
      perror("connect");
      exit(EXIT_FAILURE);
    }
  }

  return sock;
}

void error_and_exit(const char *msg) {
  fprintf(stderr, "%s\n\n", msg);
  exit(EXIT_FAILURE);
}

void print_cmp_obj (cmp_object_t obj, int fh) {
  char sbuf[1024];
  switch (obj.type) {
    case CMP_TYPE_POSITIVE_FIXNUM:
    case CMP_TYPE_UINT8:
      printf("Unsigned Integer: %u\n", obj.as.u8);
      break;
    case CMP_TYPE_FIXMAP:
    case CMP_TYPE_MAP16:
    case CMP_TYPE_MAP32:
      printf("Map: %u\n", obj.as.map_size);
      break;
    case CMP_TYPE_FIXARRAY:
    case CMP_TYPE_ARRAY16:
    case CMP_TYPE_ARRAY32:
      //for (int i = 0; i < obj.as.array_size; i++) {
      //  print_cmp_obj()
      printf("Array: %u\n", obj.as.array_size);
      break;
    case CMP_TYPE_FIXSTR:
    case CMP_TYPE_STR8:
    case CMP_TYPE_STR16:
    case CMP_TYPE_STR32:
      if (!read_bytes(sbuf, obj.as.str_size, fh))
        error_and_exit(strerror(errno));
      sbuf[obj.as.str_size] = 0;
      printf("String: \"%s\"\n", sbuf);
      break;
    case CMP_TYPE_BIN8:
    case CMP_TYPE_BIN16:
    case CMP_TYPE_BIN32:
      memset(sbuf, 0, sizeof(sbuf));
      if (!read_bytes(sbuf, obj.as.bin_size, fh))
        error_and_exit(strerror(errno));
      printf("Binary: %s\n", sbuf);
      break;
    case CMP_TYPE_NIL:
      printf("NULL\n");
      break;
    case CMP_TYPE_BOOLEAN:
      if (obj.as.boolean)
        printf("Boolean: true\n");
      else
        printf("Boolean: false\n");
      break;
    case CMP_TYPE_EXT8:
    case CMP_TYPE_EXT16:
    case CMP_TYPE_EXT32:
    case CMP_TYPE_FIXEXT1:
    case CMP_TYPE_FIXEXT2:
    case CMP_TYPE_FIXEXT4:
    case CMP_TYPE_FIXEXT8:
    case CMP_TYPE_FIXEXT16:
      printf("Extended type {%d, %u}: ", obj.as.ext.type, obj.as.ext.size);
      while (obj.as.ext.size--) {
        read_bytes(sbuf, sizeof(uint8_t), fh);
        printf("%02x ", sbuf[0]);
      }
      printf("\n");
      break;
    case CMP_TYPE_FLOAT:
      printf("Float: %f\n", obj.as.flt);
      break;
    case CMP_TYPE_DOUBLE:
      printf("Double: %f\n", obj.as.dbl);
      break;
    case CMP_TYPE_UINT16:
      printf("Unsigned Integer: %u\n", obj.as.u16);
      break;
    case CMP_TYPE_UINT32:
      printf("Unsigned Integer: %u\n", obj.as.u32);
      break;
    case CMP_TYPE_UINT64:
      printf("Unsigned Integer: %llu!\n", obj.as.u64);
      break;
    case CMP_TYPE_NEGATIVE_FIXNUM:
    case CMP_TYPE_SINT8:
      printf("Signed Integer: %d\n", obj.as.s8);
      break;
    case CMP_TYPE_SINT16:
      printf("Signed Integer: %d\n", obj.as.s16);
      break;
    case CMP_TYPE_SINT32:
      printf("Signed Integer: %d\n", obj.as.s32);
      break;
    case CMP_TYPE_SINT64:
      printf("Signed Integer: %lld!\n", obj.as.s64);
      break;
    default:
      printf("Unrecognized object type %u\n", obj.type);
      break;
  }
}
