#include <string.h>
#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

typedef struct
{
  void *zmq_context;
  void *zmq_socket;
} cipc_zmq_private;

static int
cipc_zmq_init (void **context, void *config)
{
  cipc_zmq_config *zmq_config = (cipc_zmq_config *)config;

  cipc_zmq_private *zctx = malloc (sizeof (cipc_zmq_private));
  if (!zctx)
    {
      fprintf (stderr, "Failed to alloc memory for zctx\n");

      return -1;
    }

  zctx->zmq_context = zmq_ctx_new ();
  if (!zctx->zmq_context)
    {
      fprintf (stderr, "Failed to create zmq context\n");

      free (zctx);

      return -1;
    }

  zctx->zmq_socket = zmq_socket (zctx->zmq_context, zmq_config->socket);
  if (!zctx->zmq_socket)
    {
      fprintf (stderr, "Failed to create ZMQ socket\n");

      zmq_ctx_destroy (zctx->zmq_context);

      free (zctx);

      return -1;
    }

  zmq_setsockopt (zctx->zmq_socket, ZMQ_SNDTIMEO, &zmq_config->timeout,
                  sizeof (int));
  zmq_setsockopt (zctx->zmq_socket, ZMQ_RCVTIMEO, &zmq_config->timeout,
                  sizeof (int));
  zmq_setsockopt (zctx->zmq_socket, ZMQ_LINGER, &zmq_config->linger,
                  sizeof (int));
  zmq_setsockopt (zctx->zmq_socket, ZMQ_SNDHWM, &zmq_config->sndhwm,
                  sizeof (int));
  zmq_setsockopt (zctx->zmq_socket, ZMQ_RCVHWM, &zmq_config->rcvhwm,
                  sizeof (int));

  if (strncmp (zmq_config->address, "tcp://*", 7) == 0)
    {
      if (zmq_bind (zctx->zmq_socket, zmq_config->address) != 0)
        {
          fprintf (stderr, "Failed to bind: %s\n",
                   zmq_strerror (zmq_errno ()));
          zmq_close (zctx->zmq_socket);
          zmq_ctx_destroy (zctx->zmq_context);
          free (zctx);
          return -1;
        }
    }
  else
    {
      if (zmq_connect (zctx->zmq_socket, zmq_config->address) != 0)
        {
          fprintf (stderr, "Failed to connect: %s\n",
                   zmq_strerror (zmq_errno ()));
          zmq_close (zctx->zmq_socket);
          zmq_ctx_destroy (zctx->zmq_context);
          free (zctx);
          return -1;
        }
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
