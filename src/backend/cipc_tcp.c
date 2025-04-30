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
    return CIPC_BAD_TCP_SOCKET_OPT;

  if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv)) < 0)
    return CIPC_BAD_TCP_SOCKET_OPT;

  return CIPC_OK;
}

static cipc_err
cipc_tcp_init (void **context, const void *config)
{
  if (!context || !config)
    return CIPC_NULL_PTR;

  cipc_tcp_config *tcp_config = (cipc_tcp_config *)config;
  cipc_tcp_private *tctx = malloc (sizeof (cipc_tcp_private));
  if (!tctx)
    return CIPC_BAD_ALLOC;

  tctx->sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (tctx->sockfd < 0)
    {
      free (tctx);

      return CIPC_BAD_TCP_SOCKET;
    }

  if (set_socket_timeout (tctx->sockfd, tcp_config->timeout) != CIPC_OK)
    {
      close (tctx->sockfd);

      free (tctx);

      return CIPC_BAD_TCP_SOCKET_OPT;
    }

  struct sockaddr_in addr;
  memset (&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons (tcp_config->port);

  addr.sin_port = htons (tcp_config->port);

  if (tcp_config->host && strcmp (tcp_config->host, "*") == 0)
    {
      addr.sin_addr.s_addr = INADDR_ANY;
      if (bind (tctx->sockfd, (struct sockaddr *)&addr, sizeof (addr)) < 0)
        {
          fprintf (stderr, "Bind failed: %s\n", strerror (errno));
          close (tctx->sockfd);
          free (tctx);
          return CIPC_BAD_TCP_BIND;
        }

      if (listen (tctx->sockfd, tcp_config->backlog) < 0)
        {
          fprintf (stderr, "Listen failed: %s\n", strerror (errno));
          close (tctx->sockfd);
          free (tctx);
          return CIPC_BAD_TCP_LISTEN;
        }
      tctx->is_server = 1;
    }
  else
    {
      if (inet_pton (AF_INET, tcp_config->host, &addr.sin_addr) <= 0)
        {
          fprintf (stderr, "Invalid address: %s\n", tcp_config->host);
          close (tctx->sockfd);
          free (tctx);
          return CIPC_BAD_TCP_ADDRESS;
        }

      if (connect (tctx->sockfd, (struct sockaddr *)&addr, sizeof (addr)) < 0)
        {
          fprintf (stderr, "Connect failed: %s\n", strerror (errno));
          close (tctx->sockfd);
          free (tctx);
          return CIPC_BAD_TCP_CONNECT;
        }
      tctx->is_server = 0;
    }

  *context = tctx;

  return CIPC_OK;
}

static cipc_err
cipc_tcp_send (void *context, const char *data, size_t length)
{
  cipc_tcp_private *tctx = (cipc_tcp_private *)context;

  ssize_t sent = send (tctx->sockfd, data, length, 0);
  if (sent < 0)
    {
      fprintf (stderr, "Send failed: %s\n", strerr (errno));

      return CIPC_BAD_TCP_SEND;
    }

  if ((size_t)sent != length)
    return CIPC_BAD_TCP_SEND;

  return CIPC_OK;
}

static cipc_err
cipc_tcp_recv (void *context, char *buffer, size_t length)
{
  cipc_tcp_private *tctx = (cipc_tcp_private *)context;

  ssize_t rcvd = recv (tctx->sockfd, buffer, length - 1, 0);
  if (rcvd < 0)
    {
      fprintf (stderr, "Recv failed: %s\n", strerror (errno));

      return CIPC_BAD_TCP_RECV;
    }

  if (rcvd == 0)
    return CIPC_BAD_TCP_RECV;

  buffer[rcvd] = '\0';

  return CIPC_OK;
}

static void
cipc_tcp_free (void *context)
{
  cipc_tcp_private *tctx = (cipc_tcp_private *)context;
  if (tctx)
    {
      shutdown (tctx->sockfd, SHUT_RDWR);

      close (tctx->sockfd);

      free (tctx);
    }
}

cipc *
cipc_create_tcp (void)
{
  cipc *instance = malloc (sizeof (cipc));
  if (!instance)
    return NULL;

  instance->init = cipc_tcp_init;
  instance->send = cipc_tcp_send;
  instance->recv = cipc_tcp_recv;
  instance->free = cipc_tcp_free;
  instance->context = NULL;

  return instance;
}
