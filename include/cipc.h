#ifndef CIPC_H
#define CIPC_H

typedef struct cipc cipc;

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
