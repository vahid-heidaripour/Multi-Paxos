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

group_address
get_server_address_by_role(const char *config, const char *role)
{ 
  group_address addr;
  FILE *infile;
  infile = fopen(config, "r");
  if (!infile)
    err(1, "\nError opening file\n");
  
  while (!feof(infile))
    { 
      fscanf(infile, "%s %s %d", addr.role, addr.ip_addr, &addr.port);
      if (strcmp(addr.role, role) == 0)
        break;
    }
  
  fclose(infile);

  return addr;
}

int 
create_socket_with_bind(const char *config, const char *role, int bind_value)
{
  u_int loop = 1;
  struct sockaddr_in addr;
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    err(1, "\ncreate socket error\n");

  group_address endpoint_addr = get_server_address_by_role(config, role);
  
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(endpoint_addr.ip_addr);
  addr.sin_port = htons(endpoint_addr.port);

  if (bind_value)
  {
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &loop, sizeof(loop)) < 0)
        err(1, "\nreuse address failed\n");

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        err(1, "\nbind error\n");
  }

  return fd;
}

int
create_socket_by_role(const char *config, const char *role, struct sockaddr_in *addr)
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    err(1, "\ncreate socket error\n");

  group_address endpoint_addr = get_server_address_by_role(config, role);
  
  memset(addr, 0, sizeof(struct sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(endpoint_addr.ip_addr);
  addr->sin_port = htons(endpoint_addr.port);

  return fd;
}

void
subscribe_multicast_group_by_role(const char *config, const char *role, const int sock_fd)
{
  group_address endpoint_addr = get_server_address_by_role(config, role);
  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr(endpoint_addr.ip_addr);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    err(1, "\nmembership failed\n");
}

void 
send_paxos_message(int sock_fd, struct sockaddr_in *dest_addr, paxos_message *msg)
{
    int send_bytes = sendto(sock_fd, msg, sizeof(paxos_message), 0, (struct sockaddr *)dest_addr, sizeof(struct sockaddr));
    if (send_bytes <= 0)
        err(1, "\nthere is problem in sender!\n");

    if (send_bytes != sizeof(paxos_message))
        err(1, "\ncould not send completely\n");
}

void 
send_paxos_prepare(int sock_fd, struct sockaddr_in *dest_addr, paxos_prepare *pp)
{
    paxos_message msg;
    msg.type = PAXOS_PREPARE;
    msg.u.prepare = *pp;
    send_paxos_message(sock_fd, dest_addr, &msg);
    /*printf("Send prepare for instance id %d and ballot %d\n", pp->instance_id, pp->ballot);*/
}

void 
send_paxos_promise(int sock_fd, struct sockaddr_in *dest_addr, paxos_promise *pp)
{
    paxos_message msg;
    msg.type = PAXOS_PROMISE;
    msg.u.promise = *pp;
    send_paxos_message(sock_fd, dest_addr, &msg);
    /*printf("Send promise for instance id %d and ballot %d\n", pp->instance_id, pp->ballot);*/
}

void 
send_paxos_accept(int sock_fd, struct sockaddr_in *dest_addr, paxos_accept *pa)
{
    paxos_message msg;
    msg.type = PAXOS_ACCEPT;
    msg.u.accept = *pa;
    send_paxos_message(sock_fd, dest_addr, &msg);
    /*printf("Send accept for instance id %d and ballot %d\n", pa->instance_id, pa->ballot);*/
}

void 
send_paxos_accepted(int sock_fd, struct sockaddr_in *dest_addr, paxos_accepted *pa)
{
    paxos_message msg;
    msg.type = PAXOS_ACCEPTED;
    msg.u.accepted = *pa;
    send_paxos_message(sock_fd, dest_addr, &msg);
    /*printf("Send accepted for instance id %d and ballot %d\n", pa->instance_id, pa->ballot);*/
}

void 
send_paxos_nack(int sock_fd, struct sockaddr_in *dest_addr, paxos_nack *pn)
{
    paxos_message msg;
    msg.type = PAXOS_NACK;
    msg.u.nack = *pn;
    send_paxos_message(sock_fd, dest_addr, &msg);
}

void 
send_paxos_submit(int sock_fd, struct sockaddr_in *dest_addr, char *data, int instance_id)
{
    paxos_message msg;
    msg.type = PAXOS_CLIENT_VALUE;
    msg.u.client_value.instance_id = instance_id;
    strcpy(msg.u.client_value.value.paxos_value_val, data);
    send_paxos_message(sock_fd, dest_addr, &msg);
    /*printf("Send submit for value %s\n", msg.u.client_value.value.paxos_value_val);*/
}

void
send_paxos_holes(int sock_fd, struct sockaddr_in *dest_addr, paxos_has_hole *ph)
{
    paxos_message msg;
    msg.type = PAXOS_HAS_HOLE;
    msg.u.has_holes.instance_id_low = ph->instance_id_low;
    msg.u.has_holes.instance_id_high = ph->instance_id_high;
    send_paxos_message(sock_fd, dest_addr, &msg);
}
