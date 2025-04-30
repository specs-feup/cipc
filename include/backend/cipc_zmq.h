#ifndef CIPC_BACKEND_ZMQ_H
#define CIPC_BACKEND_ZMQ_H

#include "cipc.h"

typedef struct
{
  const char *address;
  
  int socket;
  int timeout;
  int retries;
  int linger;
  int sndhwm;
  int rcvhwm;
} cipc_zmq_config;

cipc *cipc_create_zmq (void);

#endif // CIPC_BACKEND_ZMQ_H
