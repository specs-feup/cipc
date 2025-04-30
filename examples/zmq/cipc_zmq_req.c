#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

#define CLIENT_ADDRESS "tcp://localhost:5555"
#define CLIENT_BUFFER_SIZE 1024
#define CLIENT_MESSAGE "Hello from Client!"

static void
client_free (cipc **client)
{
  if (client && *client)
    {
      cipc_free (*client);
      *client = NULL;
    }
}

static int
client_init (cipc **client, const cipc_zmq_config *config)
{
  *client = cipc_create (CIPC_PROTOCOL_ZMQ);
  if (!(*client))
    {
      fprintf (stderr, "Failed to create client instance!\n");

      return EXIT_FAILURE;
    }

  if ((*client)->init (&(*client)->context, config) != CIPC_OK)
    {
      fprintf (stderr, "Failed to initialize client!\n");

      client_free (client);

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

int
main (void)
{
  cipc_zmq_config *config = cipc_zmq_config_req (CLIENT_ADDRESS);
  if (!config)
    {
      fprintf (stderr, "Failed to create client config!\n");

      return EXIT_FAILURE;
    }

  cipc *client = NULL;
  char buffer[CLIENT_BUFFER_SIZE] = { 0 };

  int result = EXIT_FAILURE;

  if (client_init (&client, config) == EXIT_SUCCESS)
    {
      if (client->send (client->context, CLIENT_MESSAGE, strlen (CLIENT_MESSAGE + 1)) != CIPC_OK)
        {
          fprintf (stderr, "Failed to send message: %s\n", CLIENT_MESSAGE);
        }
      else if (client->recv (client->context, buffer, sizeof (buffer)) != CIPC_OK)
        {
          fprintf (stderr, "Failed to receive response!\n");
        }
      else
        {
          fprintf (stdout, "[REQ Client] Received: %s\n", buffer);

          result = EXIT_SUCCESS;
        }
    }

  client_free (&client);

  free (config);

  return result;
}