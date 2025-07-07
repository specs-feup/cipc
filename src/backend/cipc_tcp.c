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
set_socket_timeouts (int sockfd, int sndtimeo, int rcvtimeo)
{
  struct timeval tv_snd, tv_rcv;

  tv_snd.tv_sec = sndtimeo / 1000;
  tv_snd.tv_usec = (sndtimeo % 1000) * 1000;

  tv_rcv.tv_sec = rcvtimeo / 1000;
  tv_rcv.tv_usec = (rcvtimeo % 1000) * 1000;

  if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_snd, sizeof (tv_snd)) < 0)
    return CIPC_BAD_TCP_SOCKET_OPT;

  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_rcv, sizeof (tv_rcv)) < 0)
    return CIPC_BAD_TCP_SOCKET_OPT;

  return CIPC_OK;
}

static cipc_err
connect_with_retries (int sockfd, const struct sockaddr *addr, socklen_t addrlen, int retries)
{
  int retry_count = 0;
  int retry_delay_ms = 100;

  while (1)
    {
      if (connect (sockfd, addr, addrlen) == 0)
        {
          return CIPC_OK;
        }

      if (retry_count >= retries)
        {
          return CIPC_BAD_TCP_CONNECT;
        }

      retry_count++;

      usleep (retry_delay_ms * 1000);
      retry_delay_ms = retry_delay_ms * 2 > 5000 ? 5000 : retry_delay_ms * 2;
    }
}

static cipc_err
cipc_tcp_init (void **context, const void *config)
{
  if (!context || !config)
    return CIPC_NULL_PTR;

  const cipc_tcp_config *cfg = (const cipc_tcp_config *)config;
  cipc_tcp_private *tctx = malloc (sizeof (cipc_tcp_private));
  if (!tctx)
    return CIPC_BAD_ALLOC;

  tctx->sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (tctx->sockfd < 0)
    {
      free (tctx);
      return CIPC_BAD_TCP_SOCKET;
    }

  cipc_err err = set_socket_timeouts (tctx->sockfd, cfg->sockopt_sndtimeo, cfg->sockopt_rcvtimeo);
  if (err != CIPC_OK)
    {
      close (tctx->sockfd);
      free (tctx);
      return err;
    }

  struct sockaddr_in addr;
  memset (&addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons (cfg->port);

  if (cfg->mode == CIPC_TCP_MODE_BIND)
    {
      addr.sin_addr.s_addr = INADDR_ANY;

      if (bind (tctx->sockfd, (struct sockaddr *)&addr, sizeof (addr)) < 0)
        {
          fprintf (stderr, "Bind failed: %s\n", strerror (errno));
          close (tctx->sockfd);
          free (tctx);
          return CIPC_BAD_TCP_BIND;
        }

      if (listen (tctx->sockfd, cfg->backlog) < 0)
        {
          fprintf (stderr, "Listen failed: %s\n", strerror (errno));
          close (tctx->sockfd);
          free (tctx);
          return CIPC_BAD_TCP_LISTEN;
        }

      int client_fd = accept (tctx->sockfd, NULL, NULL);
      if (client_fd < 0)
        {
          fprintf (stderr, "Accept failed: %s\n", strerror (errno));
          close (tctx->sockfd);
          free (tctx);
          return CIPC_BAD_TCP_SOCKET;
        }

      close (tctx->sockfd);

      tctx->sockfd = client_fd;
      tctx->is_server = 1;
    }
  else
    {
      if (inet_pton (AF_INET, cfg->host, &addr.sin_addr) <= 0)
        {
          fprintf (stderr, "Invalid address: %s\n", cfg->host);
          close (tctx->sockfd);
          free (tctx);
          return CIPC_BAD_TCP_ADDRESS;
        }

      err = connect_with_retries (tctx->sockfd, (struct sockaddr *)&addr, sizeof (addr),
                                  cfg->sockopt_retries);
      if (err != CIPC_OK)
        {
          fprintf (stderr, "Connect failed after %d retries: %s\n", cfg->sockopt_retries,
                   strerror (errno));
          close (tctx->sockfd);
          free (tctx);
          return err;
        }

      tctx->is_server = 0;
    }

  *context = tctx;
  return CIPC_OK;
}

static cipc_err
cipc_tcp_send (void *context, const char *data, size_t length, size_t *len_out)
{
  cipc_tcp_private *tctx = (cipc_tcp_private *)context;

  // WARNING: len_out is deprecated!
  if (len_out != NULL) *len_out = 0;

  ssize_t sent = send (tctx->sockfd, data, length, 0);
  if (sent < 0)
    {
      fprintf (stderr, "Send failed: %s\n", strerror (errno));

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
