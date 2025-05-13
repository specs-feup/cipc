#ifndef CIPC_H
#define CIPC_H

#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  CIPC_OK = 0,
  CIPC_BAD_ALLOC,
  CIPC_BAD_ZMQ_CONTEXT,
  CIPC_BAD_ZMQ_SOCKET,
  CIPC_BAD_ZMQ_BIND,
  CIPC_BAD_ZMQ_CONNECT,
  CIPC_BAD_ZMQ_SEND,
  CIPC_BAD_ZMQ_RECV,
  CIPC_BAD_TCP_SOCKET,
  CIPC_BAD_TCP_BIND,
  CIPC_BAD_TCP_LISTEN,
  CIPC_BAD_TCP_ADDRESS,
  CIPC_BAD_TCP_CONNECT,
  CIPC_BAD_TCP_SEND,
  CIPC_BAD_TCP_RECV,
  CIPC_BAD_TCP_SOCKET_OPT,
  CIPC_NULL_PTR,
} cipc_err;

typedef struct cipc
{
  cipc_err (*init) (void **context, const void *config);
  cipc_err (*send) (void *context, const char *data, size_t length);
  cipc_err (*recv) (void *context, char *buffer, size_t length);
  void (*free) (void *context);

  void *context;
} cipc;

typedef enum
{
  CIPC_PROTOCOL_ZMQ,
  CIPC_PROTOCOL_TCP,
  CIPC_PROTOCOL_GRPC
} cipc_protocol;

cipc *cipc_create (cipc_protocol protocol);

void cipc_free (cipc *instance);

#ifdef __cplusplus
}
#endif

#endif // CIPC_H
