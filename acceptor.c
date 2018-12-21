#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <event2/event.h>
#include <assert.h>
#include <err.h>
#include "communication_layer.h"
#include "utils.h"

#define MSGBUFSIZE 1024

int id;

struct sockaddr_in proposer_addr;
struct sockaddr_in learner_addr;

int proposer_sock_fd, learner_sock_fd;
int next_instance_id = 1;
int free_position = 0;

struct instance 
{
  int instance_id;
  int ballot;
  int value_ballot;
  paxos_value value;
};
struct instance instance_array[1024];

struct instance 
set_default_values()
{
  struct instance ins;
  ins.instance_id = -1;
  ins.ballot = -1;
  ins.value_ballot = -1;
  
  return ins;
}

struct instance 
instance_new(int instance_id, int ballot)
{
  struct instance ins;
  ins.instance_id = instance_id;
  ins.ballot = ballot;
  ins.value_ballot = 0;
  assert(ins.instance_id >= 0);
  return ins;
}

struct acceptor
{
  int id;
};

struct acceptor
*acceptor_new(int id)
{
  struct acceptor *a;
  a = malloc(sizeof(struct acceptor));
  a->id = id;
  return a;
}

void
acceptor_free(struct acceptor *a)
{
  if(a)
    free(a);
}

void 
unbox_received_message(paxos_message msg)
{
  paxos_message_type mst = (paxos_message_type)msg.type;
  switch (mst)
  {
    case PAXOS_PREPARE:
    {
      uint32_t instacne_id = msg.u.prepare.instance_id;
      uint32_t ballot = msg.u.prepare.ballot;

      struct instance inst;
      if (instance_array[instacne_id].instance_id < 0)
      {
        inst = instance_new(next_instance_id++, 0);
        instance_array[free_position++] = inst;
      }
      else 
        inst = instance_array[instacne_id];

      if (ballot >= instance_array[instacne_id].ballot)
      {
        instance_array[instacne_id].ballot = ballot;
        paxos_promise pp;
        pp.acceptor_id = id;
        pp.instance_id = instacne_id;
        pp.ballot = ballot;
        pp.value_ballot = instance_array[instacne_id].value_ballot;
        pp.value = instance_array[instacne_id].value;
        send_paxos_promise(proposer_sock_fd, &proposer_addr, &pp);
      }
      //else send nack

    }
    break;

    case PAXOS_PROMISE:
    break;

    case PAXOS_ACCEPT:
    {
      paxos_accept pa = msg.u.accept;
      instance_array[pa.instance_id].instance_id = pa.instance_id;
      instance_array[pa.instance_id].ballot = pa.ballot;
      instance_array[pa.instance_id].value = pa.value;

      paxos_accepted pac;
      pac.acceptor_id = id;
      pac.instance_id = pa.instance_id;
      pac.ballot = pa.ballot;
      pac.value = pa.value;
      send_paxos_accepted(proposer_sock_fd, &proposer_addr, &pac);
    }
    break;

    case PAXOS_ACCEPTED:
    break;

    case PAXOS_CLIENT_VALUE:
    break;

    default:
    break;
  }
}

void 
on_receive_message(evutil_socket_t fd, short event, void *arg)
{
  struct sockaddr_in remote;
  socklen_t addrlen = sizeof(remote);
  paxos_message result;
  int revc_bytes = recvfrom(fd, &result, sizeof(result), 0, (struct sockaddr *)&remote, &addrlen);
  if (revc_bytes < 0)
    err(1, "\nthere is a problem in recvfrom in acceptor\n");

  if (revc_bytes != sizeof(result))
    err(1, "\ncorrupted data in acceptor\n");

  unbox_received_message(result);
}

int 
main(int argc, char *argv[])
{
  if (argc < 2)
    err(1, "\n2 arguments needed!\n");

  for (int i = 0; i < 1024; ++i)
    instance_array[i] = set_default_values();

  id = atoi(argv[1]);
  struct acceptor *acceptor_instance = acceptor_new(id);

  if (acceptor_instance)
  {
    struct event_base *base = NULL;
    base = event_base_new();

    struct sockaddr_in acceptor_addr;

    int acceptor_socket = create_socket_with_bind("acceptors", acceptor_addr, 1);
    evutil_make_socket_nonblocking(acceptor_socket);
    subscribe_multicast_group_by_role("acceptors", acceptor_socket);

    proposer_sock_fd = create_socket_by_role("proposers", &proposer_addr);
    learner_sock_fd = create_socket_by_role("learners", &learner_addr);

    struct event *ev_receive_message;
    ev_receive_message = event_new(base, acceptor_socket, EV_READ|EV_PERSIST, on_receive_message, &base);
    event_add(ev_receive_message, NULL);

    event_base_dispatch(base);

    event_free(ev_receive_message);
    event_base_free(base);

    return 0;
  }

  return 1;
}
