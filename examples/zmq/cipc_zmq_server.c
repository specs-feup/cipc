#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cipc.h"

int
main (void)
{
  cipc *server = cipc_create (CIPC_PROTOCOL_ZMQ);
  if (!server)
    {
      fprintf (stderr, "Failed to create server\n");

      return EXIT_FAILURE;
    }

  if (server->init (&server->context, "tcp://*:5555") != 0)
    {
      fprintf (stderr, "Failed to init server\n");

      cipc_free (server);

      return EXIT_FAILURE;
    }

  printf ("Server is running and waiting for messages...\n");

  char buffer[1024];

  int received = server->recv (server->context, buffer, sizeof (buffer));
  if (received < 0)
    {
      fprintf (stderr, "Failed to receive message\n");

      cipc_free (server);

      return EXIT_FAILURE;
    }

  printf ("Server received: %s\n", buffer);

  const char *response = "Hello from server!";
  if (server->send (server->context, response, strlen (response)) != 0)
    {
      fprintf (stderr, "Failed to send response\n");

      cipc_free (server);

      return EXIT_FAILURE;
    }

  cipc_free (server);

  return EXIT_SUCCESS;
}
