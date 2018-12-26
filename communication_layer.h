#ifndef COMMUNICATION_LAYER_H
#define COMMUNICATION_LAYER_H

#include "utils.h"

struct group_address
{ 
  char role[20];
  char ip_addr[20];
  int port;
};
typedef struct group_address group_address;

group_address
get_server_address_by_role(const char *config, const char *role);

int 
create_socket_with_bind(const char *config, const char *role, int bind_value);

int
create_socket_by_role(const char *config, const char *role, struct sockaddr_in *addr);

void 
subscribe_multicast_group_by_role(const char *config, const char *role, const int sock_id);

void 
send_paxos_message(int sock_fd, struct sockaddr_in *dest_addr, paxos_message *msg);

void 
send_paxos_prepare(int sock_fd,	struct sockaddr_in *dest_addr, paxos_prepare *pp);

void 
send_paxos_promise(int sock_fd, struct sockaddr_in *dest_addr, paxos_promise *pp);

void 
send_paxos_accept(int sock_fd, struct sockaddr_in *dest_addr, paxos_accept *pa);

void 
send_paxos_accepted(int sock_fd, struct sockaddr_in *dest_addr, paxos_accepted *pa);

void 
send_paxos_nack(int sock_fd, struct sockaddr_in *dest_addr, paxos_nack *pn);

void
send_paxos_holes(int sock_fd, struct sockaddr_in *dest_addr, paxos_has_hole *ph);

void 
send_paxos_submit(int sock_fd, struct sockaddr_in *dest_addr, char *data, int instance_id);

#endif