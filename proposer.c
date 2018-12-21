#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <event2/event.h>
#include <pthread.h>
#include <assert.h>
#include <err.h>
#include "communication_layer.h"
#include "utils.h"

#define MSGBUFSIZE 1024
#define NUMBER_OF_ACCEPTORS 3

struct sockaddr_in acceptor_addr;
struct sockaddr_in learner_addr;

int acceptor_sock_fd, learner_sock_fd;
int next_instance_id = 0;
int free_position = 0;

struct instance 
{
  int instance_id;
  int ballot;
  paxos_value value;
  paxos_value promised_value;
  int value_ballot;
};
struct instance instance_array[1024];
int accpetor_promise_message[1024];
int acceptor_decided_message[1024];

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

struct proposer
{
  int id;
  int number_of_acceptors;
};

struct proposer 
*proposer_new(int id, int number_of_acceptors)
{
  struct proposer *p;
  p = malloc(sizeof(struct proposer));
  if(p)
  {
    assert(id > 0);
    p->id = id;
    p->number_of_acceptors = number_of_acceptors;

    return p;
  }

  return NULL;
}

void 
proposer_free(struct proposer *p)
{
  if(p)
    free(p);
}

int 
get_quorum()
{
  printf("quorum : %d\n", (NUMBER_OF_ACCEPTORS / 2) + 1);
  return (NUMBER_OF_ACCEPTORS / 2) + 1;
}

void 
unbox_received_message(paxos_message *msg)
{
  paxos_message_type mst = (paxos_message_type)msg->type;
  switch (mst)
  {
      case PAXOS_PREPARE:
      break;

      case PAXOS_PROMISE:
      {
        int instance_id = msg->u.promise.instance_id;
        if (accpetor_promise_message[instance_id] <= get_quorum()) 
        {
          accpetor_promise_message[instance_id]++;
          if (accpetor_promise_message[instance_id] == get_quorum())
          {
            //send accept message
            paxos_accept pa;
            pa.instance_id = instance_id;
            pa.ballot = msg->u.promise.ballot;
            strcpy(pa.value.paxos_value_val, instance_array[instance_id].value.paxos_value_val);

            accpetor_promise_message[instance_id] = 100; // don't send accept again in case that has 3 acceptors

            send_paxos_accept(acceptor_sock_fd, &acceptor_addr, &pa);
          }
        }
      }
      break;

      case PAXOS_ACCEPT:
      break;

      case PAXOS_ACCEPTED:
      {
        //send value to learners
        int instance_id = msg->u.accepted.instance_id;
        if (acceptor_decided_message[instance_id] <= get_quorum())
        {
          acceptor_decided_message[instance_id]++;
          if (acceptor_decided_message[instance_id] == get_quorum())
          {
            instance_array[instance_id].promised_value = msg->u.accepted.value;
            
            acceptor_decided_message[instance_id] = 100;

            paxos_client_value pc;  
            pc.value = instance_array[instance_id].promised_value;
            send_paxos_submit(learner_sock_fd, &learner_addr, pc.value.paxos_value_val);
          }
        }
      }
      break;

      case PAXOS_CLIENT_VALUE:
      {
        struct instance inst = instance_new(next_instance_id++, 0);
        instance_array[free_position++] = inst;
        paxos_value val;
        strcpy(val.paxos_value_val, msg->u.client_value.value.paxos_value_val);
        strcpy(inst.value.paxos_value_val, val.paxos_value_val);

        paxos_prepare pp;
        pp.instance_id = next_instance_id - 1;
        pp.ballot = 0;
        send_paxos_prepare(acceptor_sock_fd, &acceptor_addr, &pp);
      }
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
  int recv_bytes = recvfrom(fd, &result, sizeof(result), 0, (struct sockaddr *)&remote, &addrlen);
  if (recv_bytes < 0)
    err(1, "\nthere is a problem in recvfrom\n");

  if (recv_bytes != sizeof(result))
    err(1, "\ncorrupted data\n");

  //char m[1024];
  //strcpy(m, result.u.client_value.value.paxos_value_val);
  //send_paxos_submit(acceptor_sock_fd, &acceptor_addr, m, sizeof(m));
  unbox_received_message(&result);
}

int 
main(int argc, char* argv[])
{
  if (argc < 2)
    err(1, "\n2 arguments needed!\n");
    
  int number_of_acceptors = NUMBER_OF_ACCEPTORS;
  int id = atoi(argv[1]);
  struct proposer *proposer_instance = proposer_new(id, number_of_acceptors);

  if (proposer_instance)
  {
    struct event_base *base = NULL;
    base = event_base_new();

    struct sockaddr_in proposer_addr;

    int proposer_socket = create_socket_with_bind("proposers", proposer_addr, 1);
    evutil_make_socket_nonblocking(proposer_socket);
    subscribe_multicast_group_by_role("proposers", proposer_socket);

    acceptor_sock_fd = create_socket_by_role("acceptors", &acceptor_addr);
    learner_sock_fd = create_socket_by_role("learners", &learner_addr);

    struct event *ev_submit_client;
    ev_submit_client = event_new(base, proposer_socket, EV_READ|EV_PERSIST, on_receive_message, &base);
    event_add(ev_submit_client, NULL);

    event_base_dispatch(base);

    event_free(ev_submit_client);
    event_base_free(base);

    return 0;
  }

  return 1;
}
