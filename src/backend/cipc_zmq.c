#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

typedef struct
{
  void *zmq_context;
  void *zmq_socket;
} cipc_zmq_private;

static int
cipc_zmq_init (void **context, const char *config)
{
  cipc_zmq_private *zctx = malloc (sizeof (cipc_zmq_private));
  if (!zctx)
    {
      return -1; // TODO: this needs proper error handling
    }

  zctx->zmq_context = zmq_ctx_new ();
  if (!zctx->zmq_context)
    {
      free (zctx);

      return -1;
    }

  if (zmq_connect (zctx->zmq_socket, config) != 0)
    {
      zmq_close (zctx->zmq_socket);
      zmq_ctx_destroy (zctx->zmq_socket);

      free (zctx);

      return -1;
    }

  *context = zctx;

  return 0;
}

static int
cipc_zmq_send (void *context, const char *data, size_t length)
{
  cipc_zmq_private *zctx = (cipc_zmq_private *)context;

  int rc = zmq_send (zctx->zmq_socket, data, length, 0);

  return (rc >= 0) ? 0 : -1; // TODO: this needs proper error handling
}

static int
cipc_zmq_recv (void *context, char *buffer, size_t length)
{
  cipc_zmq_private *zctx = (cipc_zmq_private *)context;

  int rc = zmq_recv (zctx->zmq_socket, buffer, length - 1, 0);

  if (rc >= 0)
    {
      buffer[rc] = '\0';

      return rc;
    }

  return -1;
}

static void
cipc_zmq_free (void *context)
{
  cipc_zmq_private *zctx = (cipc_zmq_private *)context;
  if (zctx)
    {
      zmq_close (zctx->zmq_socket);
      zmq_ctx_destroy (zctx->zmq_context);

      free (zctx);
    }
}

cipc *
cipc_create_zmq (void)
{
  cipc *instance = malloc (sizeof (cipc));
  if (!instance)
    {
      return NULL;
    }

  instance->init = cipc_zmq_init;
  instance->send = cipc_zmq_send;
  instance->recv = cipc_zmq_recv;
  instance->free = cipc_zmq_free;
  instance->context = NULL;

  return instance;
}
