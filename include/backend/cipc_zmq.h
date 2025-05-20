#ifndef CIPC_BACKEND_ZMQ_H
#define CIPC_BACKEND_ZMQ_H

#include "cipc.h"

#ifdef __cplusplus
extern "C" {
#endif 

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

  int sockopt_sndtimeo;
  int sockopt_rcvtimeo;
  int sockopt_retries;
} cipc_zmq_config;

cipc *cipc_create_zmq (void);

cipc_zmq_config *cipc_zmq_config_default (const char *address, int socket_type, cipc_zmq_mode mode);

cipc_zmq_config *cipc_zmq_config_req (const char *address);
cipc_zmq_config *cipc_zmq_config_rep (const char *address);

void cipc_zmq_config_set_sndtimeo(cipc_zmq_config *config, int sndtimeo);
void cipc_zmq_config_set_rcvtimeo(cipc_zmq_config *config, int rcvtimeo);
void cipc_zmq_config_set_retries(cipc_zmq_config *config, int retries);

#ifdef __cplusplus
}
#endif

#endif // CIPC_BACKEND_ZMQ_H
