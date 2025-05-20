#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend/cipc_tcp.h"
#include "cipc.h"

#define CLIENT_HOST "127.0.0.1"
#define CLIENT_PORT 5555
#define CLIENT_BUFFER_SIZE 1024
#define CLIENT_MESSAGE "Hello from TCP Client!"

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
client_init (cipc **client, const cipc_tcp_config *config)
{
  *client = cipc_create (CIPC_PROTOCOL_TCP);
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
  cipc_tcp_config config = {
    .host = CLIENT_HOST,
    .port = CLIENT_PORT,
    .mode = CIPC_TCP_MODE_CONNECT,
    .sockopt_sndtimeo = 5000,
    .sockopt_rcvtimeo = 5000,
    .sockopt_retries = 3,
    .backlog = 0,
  };

  cipc *client = NULL;
  char buffer[CLIENT_BUFFER_SIZE] = { 0 };
  int result = EXIT_FAILURE;

  if (client_init (&client, &config) == EXIT_SUCCESS)
    {
      if (client->send (client->context, CLIENT_MESSAGE, strlen (CLIENT_MESSAGE)) != CIPC_OK)
        {
          fprintf (stderr, "Failed to send message: %s\n", CLIENT_MESSAGE);
        }
      else if (client->recv (client->context, buffer, sizeof (buffer)) != CIPC_OK)
        {
          fprintf (stderr, "Failed to receive response!\n");
        }
      else
        {
          fprintf (stdout, "[TCP Client] Received: %s\n", buffer);

          result = EXIT_SUCCESS;
        }
    }

  client_free (&client);

  return result;
}
