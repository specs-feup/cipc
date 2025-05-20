#ifndef CIPC_TCP_H
#define CIPC_TCP_H

#include "cipc.h"

typedef enum
{
  CIPC_TCP_MODE_BIND,
  CIPC_TCP_MODE_CONNECT
} cipc_tcp_mode;

typedef struct
{
  const char *host;

  int port;
  
  cipc_tcp_mode mode;
  
  int sockopt_sndtimeo;
  int sockopt_rcvtimeo;
  int sockopt_retries;

  int backlog;
} cipc_tcp_config;

cipc *cipc_create_tcp (void);

#endif // CIPC_TCP_H
