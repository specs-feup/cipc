#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "backend/cipc_tcp.h"
#include "cipc.h"

typedef struct
{
  int sockfd;
  int is_server;
} cipc_tcp_private;

static cipc_err
set_socket_timeout (int sockfd, int timeout)
{
  struct timeval tv;

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0)
    return CIPC_BAD_TCP_SOCK_OPT;

  if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv)) < 0)
    return CIPC_BAD_TCP_SOCK_OPT;

  return CIPC_OK;
}