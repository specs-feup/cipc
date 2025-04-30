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
helper_set_sockopts (void *socket, const cipc_zmq_config *config)
{
  if (!socket || !config)
    return CIPC_NULL_PTR;

  zmq_setsockopt (socket, ZMQ_LINGER, &config->sockopt_linger, sizeof (int));
  zmq_setsockopt (socket, ZMQ_SNDHWM, &config->sockopt_sndhwm, sizeof (int));
  zmq_setsockopt (socket, ZMQ_RCVHWM, &config->sockopt_rcvhwm, sizeof (int));
  zmq_setsockopt (socket, ZMQ_SNDTIMEO, &config->sockopt_sndtimeo, sizeof (int));
  zmq_setsockopt (socket, ZMQ_RCVTIMEO, &config->sockopt_rcvtimeo, sizeof (int));
  zmq_setsockopt (socket, ZMQ_RECONNECT_IVL, &config->sockopt_reconnect_ivl, sizeof (int));
  zmq_setsockopt (socket, ZMQ_RECONNECT_IVL_MAX, &config->sockopt_reconnect_ivl_max, sizeof (int));
  zmq_setsockopt (socket, ZMQ_IPV6, &config->sockopt_ipv6, sizeof (int));
  zmq_setsockopt (socket, ZMQ_TCP_KEEPALIVE, &config->sockopt_tcp_keepalive, sizeof (int));
  zmq_setsockopt (socket, ZMQ_TCP_KEEPALIVE_IDLE, &config->sockopt_tcp_keepalive_idle,
                  sizeof (int));
  zmq_setsockopt (socket, ZMQ_TCP_KEEPALIVE_CNT, &config->sockopt_tcp_keepalive_cnt, sizeof (int));
  zmq_setsockopt (socket, ZMQ_TCP_KEEPALIVE_INTVL, &config->sockopt_tcp_keepalive_intvl,
                  sizeof (int));
  zmq_setsockopt (socket, ZMQ_ROUTER_MANDATORY, &config->sockopt_router_mandatory, sizeof (int));
  zmq_setsockopt (socket, ZMQ_MAXMSGSIZE, &config->sockopt_maxmsgsize, sizeof (int));

  if (config->sockopt_identity)
    zmq_setsockopt (socket, ZMQ_IDENTITY, &config->sockopt_identity, sizeof (int));

  if (config->secopt_pubkey && config->secopt_privkey && config->secopt_svkey)
    {
      zmq_setsockopt (socket, ZMQ_CURVE_PUBLICKEY, config->secopt_pubkey, 40);
      zmq_setsockopt (socket, ZMQ_CURVE_SECRETKEY, config->secopt_privkey, 40);
      zmq_setsockopt (socket, ZMQ_CURVE_SERVERKEY, config->secopt_svkey, 40);
    }

  if (config->socket_type == ZMQ_SUB && config->topics)
    for (size_t i = 0; i < config->num_topics; i++)
      zmq_setsockopt (socket, ZMQ_SUBSCRIBE, config->topics[i], strlen (config->topics[i]));

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

  cfg->sockopt_linger = 0;
  cfg->sockopt_sndhwm = 1000;
  cfg->sockopt_rcvhwm = 1000;
  cfg->sockopt_sndtimeo = 5000;
  cfg->sockopt_rcvtimeo = 5000;
  cfg->sockopt_reconnect_ivl = 100;
  cfg->sockopt_reconnect_ivl_max = 0;
  cfg->sockopt_tcp_keepalive = 1;
  cfg->sockopt_tcp_keepalive_idle = -1;
  cfg->sockopt_tcp_keepalive_cnt = -1;
  cfg->sockopt_tcp_keepalive_intvl = -1;
  cfg->sockopt_maxmsgsize = -1;
  cfg->sockopt_router_mandatory = 0;
  cfg->sockopt_ipv6 = 0;
  cfg->sockopt_identity = 0;
  cfg->sockopt_name = NULL;

  cfg->topics = NULL;
  cfg->num_topics = 0;

  cfg->secopt_pubkey = NULL;
  cfg->secopt_privkey = NULL;
  cfg->secopt_svkey = NULL;

  return cfg;
}

cipc_zmq_config *
cipc_zmq_config_pub (const char *address)
{
  return cipc_zmq_config_default (address, ZMQ_PUB, CIPC_ZMQ_MODE_CONNECT);
}

cipc_zmq_config *
cipc_zmq_config_sub (const char *address, const char **topics, size_t num_topics)
{
  cipc_zmq_config *cfg = cipc_zmq_config_default (address, ZMQ_SUB, CIPC_ZMQ_MODE_BIND);
  if (!cfg)
    return NULL;

  cfg->topics = topics;
  cfg->num_topics = num_topics;

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
