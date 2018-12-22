#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <event2/event.h>
#include <err.h>
#include "communication_layer.h"

#define MSGBUFSIZE 1024

int socket_fd;

int 
main(int argc, char* argv[])
{
  if (argc < 2)
    err(1, "\n2 arguments needed!\n");

  int id;
  char *config_file;
  char message[1024];

  id = atoi(argv[1]);
  config_file = argv[2];
  
  struct sockaddr_in addr;
  socket_fd = create_socket_by_role(config_file, "proposers", &addr);
  while (!feof(stdin))
   {
      scanf("%[^\n]%*c", message);
      send_paxos_submit(socket_fd, &addr, message, -1);
   }

  return 0;
}
