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
#include "utils.h"

#define MSGBUFSIZE 1024

struct sockaddr_in acceptor_addr;

int acceptor_sock_fd;

struct learner
{
    int id;
    int acceptors;
    int current_instance_id;
    int hightest_instance_id;
};

struct learner
learner_new(int id)
{
    struct learner l;
    l.id = id;
    l.current_instance_id = 1;
    l.hightest_instance_id = 1;


    return l;
}

/*void
learner_set_instance_id(struct learner *l, int instance_id)
{
    if(l)
    {
        l->current_instance_id = instance_id + 1;
        l->hightest_instance_id = instance_id;
    }
}*/

void 
on_receive_message(evutil_socket_t fd, short event, void *arg)
{
    struct sockaddr_in remote;
    socklen_t addrlen = sizeof(remote);
    paxos_message result;
    int recv_bytes = recvfrom(fd, &result, sizeof(result), 0, (struct sockaddr *)&remote, &addrlen);
    if (recv_bytes < 0)
        err(1, "\nthere is a problem in recvfrom in learner\n");

    if (recv_bytes != sizeof(result))
        err(1, "\ncorrupted data in acceptor\n");

    printf("%s\n", result.u.client_value.value.paxos_value_val);
}

int 
main(int argc, char *argv[])
{
    if (argc < 2)
    err(1, "\n2 arguments needed!\n");

    int number_of_acceptors = 3;
    int id = atoi(argv[1]);
    struct learner learner_instance = learner_new(id);
    
    struct event_base *base = NULL;
    base = event_base_new();

    struct sockaddr_in learner_addr;

    int learner_socket = create_socket_with_bind("learners", learner_addr, 1);
    evutil_make_socket_nonblocking(learner_socket);
    subscribe_multicast_group_by_role("learners", learner_socket);

    struct event *ev_receive_message;
    ev_receive_message = event_new(base, learner_socket, EV_READ|EV_PERSIST, on_receive_message, &base);
    event_add(ev_receive_message, NULL);

    event_base_dispatch(base);

    event_free(ev_receive_message);
    event_base_free(base);

    acceptor_sock_fd = create_socket_by_role("acceptors", &acceptor_addr);

    return 0;
}