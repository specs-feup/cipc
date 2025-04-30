#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

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
  if (!*server)
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
server_send_message (cipc *server, const char *message)
{
  if (server->send (server->context, message, strlen (message) + 1) != CIPC_OK)
    {
      fprintf (stderr, "Failed to send message: %s\n", message);

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

static int
server_recv_message (cipc *server, char *buffer, size_t size)
{
  if (server->recv (server->context, buffer, size) != CIPC_OK)
    {
      fprintf (stderr, "Failed to receive message!\n");

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

#define SERVER_ADDRESS "tcp://*:5555"
#define SERVER_TIMEOUT 5000
#define SERVER_BUFFER_SIZE 1024

int
main (void)
{
  cipc_zmq_config config = { .address = SERVER_ADDRESS,
                             .socket = ZMQ_REP,
                             .timeout = SERVER_TIMEOUT,
                             .retries = -1,
                             .linger = 0,
                             .sndhwm = -1,
                             .rcvhwm = -1 };

  cipc *server = NULL;
  char buffer[SERVER_BUFFER_SIZE] = { 0 };
  const char *response = "Hello from server!";

  if (server_init (&server, &config) != EXIT_SUCCESS)
    return EXIT_FAILURE;

  printf ("Server is running and waiting for messages...\n");

  while (1)
    {
      if (server_recv_message (server, buffer, sizeof (buffer))
          != EXIT_SUCCESS)
        {
          server_free (&server);

          return EXIT_FAILURE;
        }

      printf ("Server received: %s\n", buffer);

      if (server_send_message (server, response) != EXIT_SUCCESS)
        {
          server_free (&server);

          return EXIT_FAILURE;
        }
    }

  server_free (&server);

  return EXIT_SUCCESS;
}
