#include <string.h>
#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

#define CIPC_ZMQ_CONFIG_DEFAULT_SNDTIMEO_MS 5000
#define CIPC_ZMQ_CONFIG_DEFAULT_RCVTIMEO_MS 5000
#define CIPC_ZMQ_CONFIG_DEFAULT_RETRY_INTERVAL_MS 10
#define CIPC_ZMQ_CONFIG_DEFAULT_RETRIES 3

typedef struct
{
  void *zmq_context;
  void *zmq_socket;
} cipc_zmq_private;

static cipc_err
helper_set_sockopts (void *socket, const cipc_zmq_config *config)
{
  if (!socket || !config)
    return CIPC_NULL_PTR;

  zmq_setsockopt (socket, ZMQ_SNDTIMEO, &config->sockopt_sndtimeo, sizeof (int));
  zmq_setsockopt (socket, ZMQ_RCVTIMEO, &config->sockopt_rcvtimeo, sizeof (int));

  int retry_interval = CIPC_ZMQ_CONFIG_DEFAULT_RETRY_INTERVAL_MS;
  zmq_setsockopt (socket, ZMQ_RECONNECT_IVL, &retry_interval, sizeof (int));
  zmq_setsockopt (socket, ZMQ_RECONNECT_IVL_MAX, &config->sockopt_retries, sizeof (int));

  return CIPC_OK;
}

static cipc_err
cipc_zmq_init (void **context, const void *config)
{
  if (!context || !config)
    return CIPC_NULL_PTR;

  const cipc_zmq_config *cfg = (const cipc_zmq_config *)config;

  cipc_zmq_private *zctx = malloc (sizeof (cipc_zmq_private));
  if (!zctx)
    return CIPC_BAD_ALLOC;

  zctx->zmq_context = zmq_ctx_new ();
  if (!zctx->zmq_context)
    {
      free (zctx);

      return CIPC_BAD_ZMQ_CONTEXT;
    }

  zctx->zmq_socket = zmq_socket (zctx->zmq_context, cfg->socket_type);
  if (!zctx->zmq_socket)
    {
      zmq_ctx_destroy (zctx->zmq_context);

      free (zctx);

      return CIPC_BAD_ZMQ_SOCKET;
    }

  cipc_err err = helper_set_sockopts (zctx->zmq_socket, cfg);
  if (err != CIPC_OK)
    {
      zmq_close (zctx->zmq_socket);
      zmq_ctx_destroy (zctx->zmq_context);

      free (zctx);

      return err;
    }

  int rc = (cfg->mode == CIPC_ZMQ_MODE_BIND) ? zmq_bind (zctx->zmq_socket, cfg->address)
                                             : zmq_connect (zctx->zmq_socket, cfg->address);

  if (rc != 0)
    {
      fprintf (stderr, "ZMQ %s failed: %s\n", cfg->mode == CIPC_ZMQ_MODE_BIND ? "bind" : "connect",
               zmq_strerror (zmq_errno ()));

      zmq_close (zctx->zmq_socket);
      zmq_ctx_destroy (zctx->zmq_context);

      free (zctx);

      return (cfg->mode == CIPC_ZMQ_MODE_BIND) ? CIPC_BAD_ZMQ_BIND : CIPC_BAD_ZMQ_CONNECT;
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
cipc_zmq_recv (void *context, char *buffer, size_t length, size_t *len_out)
{
  cipc_zmq_private *zctx = (cipc_zmq_private *)context;

  int rc = zmq_recv (zctx->zmq_socket, buffer, length - 1, 0);

  if (rc >= 0)
    {
      buffer[rc] = '\0';

      if (len_out != NULL)
        *len_out = (size_t)rc;

      return CIPC_OK;
    }

  return CIPC_BAD_ZMQ_RECV;
}

void
cipc_zmq_free (void *context)
{
  if (!context)
    return;

  cipc_zmq_private *zctx = (cipc_zmq_private *)context;

  if (zctx->zmq_socket)
    zmq_close (zctx->zmq_socket);

  if (zctx->zmq_context)
    zmq_ctx_destroy (zctx->zmq_context);

  free (zctx);
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

cipc_zmq_config *
cipc_zmq_config_default (const char *address, int socket_type, cipc_zmq_mode mode)
{
  cipc_zmq_config *cfg = calloc (1, sizeof (cipc_zmq_config));
  if (!cfg)
    return NULL;

  cfg->address = address;
  cfg->socket_type = socket_type;
  cfg->mode = mode;

  cfg->sockopt_sndtimeo = CIPC_ZMQ_CONFIG_DEFAULT_SNDTIMEO_MS;
  cfg->sockopt_rcvtimeo = CIPC_ZMQ_CONFIG_DEFAULT_RCVTIMEO_MS;
  cfg->sockopt_retries = CIPC_ZMQ_CONFIG_DEFAULT_RETRIES;

  return cfg;
}

cipc_zmq_config *
cipc_zmq_config_req (const char *address)
{
  return cipc_zmq_config_default (address, ZMQ_REQ, CIPC_ZMQ_MODE_CONNECT);
}

cipc_zmq_config *
cipc_zmq_config_rep (const char *address)
{
  return cipc_zmq_config_default (address, ZMQ_REP, CIPC_ZMQ_MODE_BIND);
}

void
cipc_zmq_config_set_sndtimeo (cipc_zmq_config *config, int sndtimeo)
{
  if (!config)
    return;

  config->sockopt_sndtimeo = sndtimeo;
}

void
cipc_zmq_config_set_rcvtimeo (cipc_zmq_config *config, int rcvtimeo)
{
  if (!config)
    return;

  config->sockopt_rcvtimeo = rcvtimeo;
}

void
cipc_zmq_config_set_retries (cipc_zmq_config *config, int retries)
{
  if (!config)
    return;

  config->sockopt_retries = retries;
}
