#include "cipc.h"

#include <stdio.h>
#include <stdlib.h>

struct cipc
{
  int (*init) (void **context, const char *config);
  int (*send) (void *context, const char *data, size_t length);
  int (*recv) (void *context, char *buffer, size_t length);
  void (*free) (void *context);

  void *context;
};

cipc *
cipc_create (cipc_protocol protocol)
{
  switch (protocol)
    {
    case CIPC_PROTOCOL_ZMQ:
    case CIPC_PROTOCOL_TCP:
    case CIPC_PROTOCOL_GRPC:
    default:
      return NULL;
    }
}

void
cipc_free (cipc *instance)
{
  if (instance)
    {
      if (instance->free)
        {
          instance->free (instance->context);
        }
      free (instance);
    }
}
