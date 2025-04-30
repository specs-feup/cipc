#ifndef CIPC_TCP_H
#define CIPC_TCP_H

#include "cipc.h"

typedef struct
{
  const char *host;

  int port;
  int timeout;
  int backlog;
} cipc_tcp_config;

cipc *cipc_create_tcp (void);

#endif // CIPC_TCP_H
