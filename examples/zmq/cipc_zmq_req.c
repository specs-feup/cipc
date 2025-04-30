#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#include "backend/cipc_zmq.h"
#include "cipc.h"

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
  if (!*client)
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

static int
client_send_message (cipc *client, const char *message)
{
  if (client->send (client->context, message, strlen (message) + 1) != CIPC_OK)
    {
      fprintf (stderr, "Failed to send message: %s\n", message);

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

static int
client_recv_message (cipc *client, char *buffer, size_t size)
{
  if (client->recv (client->context, buffer, size) != CIPC_OK)
    {
      fprintf (stderr, "Failed to receive response!\n");

      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

#define CLIENT_BUFFER_SIZE 1024
#define CLIENT_ADDRESS "tcp://localhost:5555"
#define CLIENT_TIMEOUT 5000

int
main (void)
{
  const cipc_zmq_config config = { .address = CLIENT_ADDRESS,
                                   .socket = ZMQ_REQ,
                                   .timeout = CLIENT_TIMEOUT,
                                   .retries = -1,
                                   .linger = 0,
                                   .sndhwm = -1,
                                   .rcvhwm = -1 };

  cipc *client = NULL;
  char buffer[CLIENT_BUFFER_SIZE] = { 0 };
  const char *message = "Hello from client!";

  if (client_init (&client, &config) != EXIT_SUCCESS)
    return EXIT_FAILURE;

  if (client_send_message (client, message) != EXIT_SUCCESS)
    {
      client_free (&client);

      return EXIT_FAILURE;
    }

  if (client_recv_message (client, buffer, sizeof (buffer)) != EXIT_SUCCESS)
    {
      client_free (&client);

      return EXIT_FAILURE;
    }

  printf ("Client received: %s\n", buffer);

  client_free (&client);

  return EXIT_SUCCESS;
}