#include <string.h>
#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

typedef struct
{
  void *zmq_context;
  void *zmq_socket;
} cipc_zmq_private;

static cipc_err
cipc_zmq_init (void **context, const void *config)
{
  if (!context || !config)
    return CIPC_NULL_PTR;

  cipc_zmq_config *zmq_config = (cipc_zmq_config *)config;

  cipc_zmq_private *zctx = malloc (sizeof (cipc_zmq_private));
  if (!zctx)
    return CIPC_BAD_ALLOC;

  zctx->zmq_context = zmq_ctx_new ();
  if (!zctx->zmq_context)
    {
      free (zctx);

      return CIPC_BAD_ZMQ_CONTEXT;
    }

  zctx->zmq_socket = zmq_socket (zctx->zmq_context, zmq_config->socket);
  if (!zctx->zmq_socket)
    {
      zmq_ctx_destroy (zctx->zmq_context);

      free (zctx);

      return CIPC_BAD_ZMQ_SOCKET;
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

          return CIPC_BAD_ZMQ_BIND;
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

          return CIPC_BAD_ZMQ_CONNECT;
        }
    }

  *context = zctx;

  return CIPC_OK;
}

static cipc_err
cipc_zmq_send (void *context, const char *data, size_t length)
{
  cipc_zmq_private *zctx = (cipc_zmq_private *)context;

  int rc = zmq_send (zctx->zmq_socket, data, length, 0);

  return (rc >= 0) ? CIPC_OK : CIPC_BAD_ZMQ_SEND;
}

static cipc_err
cipc_zmq_recv (void *context, char *buffer, size_t length)
{
  cipc_zmq_private *zctx = (cipc_zmq_private *)context;

  int rc = zmq_recv (zctx->zmq_socket, buffer, length - 1, 0);

  if (rc >= 0)
    {
      buffer[rc] = '\0';

      return CIPC_OK;
    }

  return CIPC_BAD_ZMQ_RECV;
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
