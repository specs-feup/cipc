#include "cipc.h"
#include "backend/cipc_zmq.h"
#include "backend/cipc_tcp.h"

#include <stdio.h>

cipc *
cipc_create (cipc_protocol protocol)
{
  switch (protocol)
    {
    case CIPC_PROTOCOL_ZMQ:
      return cipc_create_zmq ();
    case CIPC_PROTOCOL_TCP:
      return cipc_create_tcp ();
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
