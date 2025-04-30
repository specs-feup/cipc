#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

#define SERVER_ADDRESS "tcp://*:5555"
#define SERVER_BUFFER_SIZE 1024
#define SERVER_REPLY "Hello from Server!"

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
server_init (cipc **server, const cipc_zmq_config *config)
{
  *server = cipc_create (CIPC_PROTOCOL_ZMQ);
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

  fprintf (stdout, "[REP Server] Online and waiting on %s\n", SERVER_ADDRESS);

  while (1)
    {
      if (server->recv (server->context, buffer, sizeof (buffer)) != CIPC_OK)
        {
          fprintf (stderr, "Failed to receive message!\n");

          return EXIT_FAILURE;
        }

      fprintf (stdout, "[REP Server] Received: %s\n", buffer);

      if (server->send (server->context, SERVER_REPLY, strlen (SERVER_REPLY + 1)) != CIPC_OK)
        {
          fprintf (stderr, "Failed to send response!\n");

          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}

int
main (void)
{
  cipc_zmq_config *config = cipc_zmq_config_rep (SERVER_ADDRESS);
  if (!config)
    {
      fprintf (stderr, "Failed to fetch server config!\n");

      return EXIT_FAILURE;
    }

  cipc *server = NULL;
  int result = EXIT_FAILURE;

  if (server_init (&server, config) == EXIT_SUCCESS)
    result = server_loop (server);

  server_free (&server);
  free (config);

  return result;
}
