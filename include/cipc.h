#ifndef CIPC_H
#define CIPC_H

#include <stdlib.h>

typedef struct cipc cipc;

typedef struct cipc
{
  int (*init) (void **context, const char *config);
  int (*send) (void *context, const char *data, size_t length);
  int (*recv) (void *context, char *buffer, size_t length);
  void (*free) (void *context);

  void *context;
} cipc;

typedef enum
{
  CIPC_PROTOCOL_ZMQ,
  CIPC_PROTOCOL_TCP,
  CIPC_PROTOCOL_GRPC
} cipc_protocol;

typedef enum
{
  CIPC_OK,
  CIPC_BAD_ALLOC,
} cipc_err;

cipc *cipc_create (cipc_protocol protocol);

void cipc_free (cipc *instance);

#endif // CIPC_H
