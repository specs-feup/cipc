#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cipc.h"

int
main (void)
{
  cipc *client = cipc_create (CIPC_PROTOCOL_ZMQ);
  if (!client)
    {
      fprintf (stderr, "Failed to create client instance\n");

      return EXIT_FAILURE;
    }

  if (client->init (&client->context, "tcp://localhost:5555") != 0)
    {
      fprintf (stderr, "Failed to init client\n");

      return EXIT_FAILURE;
    }

  const char *message = "Hello from client!";
  if (client->send (client->context, message, strlen (message)) != 0)
    {
      fprintf (stderr, "Failed to send message\n");

      cipc_free (client);

      return EXIT_FAILURE;
    }

  char buffer[1024];

  int received = client->recv (client->context, buffer, sizeof (buffer));
  if (received < 0)
    {
      fprintf (stderr, "Failed to receive reply\n");

      cipc_free (client);

      return EXIT_FAILURE;
    }

  printf ("Client received: %s\n", buffer);

  cipc_free (client);

  return EXIT_SUCCESS;
}
