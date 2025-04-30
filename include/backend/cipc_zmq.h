#ifndef CIPC_BACKEND_ZMQ_H
#define CIPC_BACKEND_ZMQ_H

#include "cipc.h"

typedef enum
{
  CIPC_ZMQ_MODE_BIND,
  CIPC_ZMQ_MODE_CONNECT
} cipc_zmq_mode;

typedef struct
{
  const char *address;
  int socket_type;
  cipc_zmq_mode mode;

  int sockopt_linger;
  int sockopt_sndhwm;
  int sockopt_rcvhwm;
  int sockopt_sndtimeo;
  int sockopt_rcvtimeo;
  int sockopt_reconnect_ivl;
  int sockopt_reconnect_ivl_max;
  int sockopt_ipv6;
  int sockopt_identity;
  int sockopt_router_mandatory;
  int sockopt_tcp_keepalive;
  int sockopt_tcp_keepalive_idle;
  int sockopt_tcp_keepalive_cnt;
  int sockopt_tcp_keepalive_intvl;
  int sockopt_maxmsgsize;
  char *sockopt_name;

  const char **topics;
  size_t num_topics;

  const char *secopt_pubkey;
  const char *secopt_privkey;
  const char *secopt_svkey;
} cipc_zmq_config;

cipc *cipc_create_zmq (void);

cipc_zmq_config *cipc_zmq_config_default (const char *address, int socket_type, cipc_zmq_mode mode);
cipc_zmq_config *cipc_zmq_config_pub (const char *address);
cipc_zmq_config *cipc_zmq_config_sub (const char *address, const char **topics, size_t num_topics);
cipc_zmq_config *cipc_zmq_config_req (const char *address);
cipc_zmq_config *cipc_zmq_config_rep (const char *address);

#endif // CIPC_BACKEND_ZMQ_H
