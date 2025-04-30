#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

int
main (void)
{
  cipc_zmq_config config
      = (cipc_zmq_config){ .address = "tcp://localhost:5555",
                           .socket = ZMQ_REQ,
                           .timeout = 5000,
                           .retries = -1,
                           .linger = 0,
                           .sndhwm = -1,
                           .rcvhwm = -1 };

  cipc *client = cipc_create (CIPC_PROTOCOL_ZMQ);
  if (!client)
    {
      fprintf (stderr, "Failed to create client instance\n");

      return EXIT_FAILURE;
    }

  if (client->init (&client->context, &config) != 0)
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
