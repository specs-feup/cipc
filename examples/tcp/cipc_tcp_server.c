#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend/cipc_tcp.h"
#include "cipc.h"

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 5555
#define SERVER_BACKLOG 1
#define SERVER_BUFFER_SIZE 1024
#define SERVER_REPLY "Hello from TCP server!"

static void
server_free (cipc **server)
{
  if (server && *server)
    {
      cipc_free (*server);

      *server = NULL;
    }
}

static int
server_init (cipc **server, const cipc_tcp_config *config)
{
  *server = cipc_create (CIPC_PROTOCOL_TCP);
  if (!(*server))
    {
      fprintf (stderr, "Failed to create server instance!\n");

      return EXIT_FAILURE;
    }

  if ((*server)->init (&(*server)->context, config) != CIPC_OK)
    {
      fprintf (stderr, "Failed to initialize server!\n");

      server_free (server);

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

static int
server_loop (cipc *server)
{
  char buffer[SERVER_BUFFER_SIZE] = { 0 };

  fprintf (stdout, "[TCP Server] Listening on %s:%d\n", SERVER_HOST, SERVER_PORT);

  while (1)
    {
      if (server->recv (server->context, buffer, sizeof (buffer)) != CIPC_OK)
        {
          fprintf (stderr, "Failed to receive message!\n");

          return EXIT_FAILURE;
        }

      fprintf (stdout, "[TCP Server] Received: %s\n", buffer);

      if (server->send (server->context, SERVER_REPLY, strlen (SERVER_REPLY)) != CIPC_OK)
        {
          fprintf (stderr, "Failed to send message!\n");

          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}

int
main (void)
{
  cipc_tcp_config config = { .host = SERVER_HOST,
                             .port = SERVER_PORT,
                             .mode = CIPC_TCP_MODE_BIND,
                             .sockopt_sndtimeo = 5000,
                             .sockopt_rcvtimeo = 5000,
                             .sockopt_retries = 3,
                             .backlog = SERVER_BACKLOG };

  cipc *server = NULL;

  int result = EXIT_FAILURE;

  if (server_init (&server, &config) == EXIT_SUCCESS)
    result = server_loop (server);

  server_free (&server);

  return result;
}
