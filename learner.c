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
struct learner learner_instance;

struct learner
learner_new(int id)
{
    struct learner l;
    l.id = id;
    l.current_instance_id = 0;
    l.hightest_instance_id = 0;

    return l;
}

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

    int instance_id = result.u.client_value.instance_id;
    if (learner_instance.current_instance_id == instance_id)
    {
        int strcmp_ret = strcmp(result.u.client_value.value.paxos_value_val, "NULL");
        if (strcmp_ret != 0)
            printf("%s\n", result.u.client_value.value.paxos_value_val);
        fflush(stdout);
        learner_instance.current_instance_id++;
    }
    else if (learner_instance.current_instance_id < instance_id)
    {
        //there is some holes in learners
        //send to acceptors
        paxos_has_hole ph;
        ph.instance_id_low = learner_instance.current_instance_id;
        ph.instance_id_high = instance_id;
        send_paxos_holes(acceptor_sock_fd, &acceptor_addr, &ph);
    }
}

int 
main(int argc, char *argv[])
{
    if (argc < 2)
    err(1, "\n2 arguments needed!\n");

    u_int id = atoi(argv[1]);

    char *config_file = argv[2];

    learner_instance = learner_new(id);
    
    struct event_base *base = NULL;
    base = event_base_new();

    int learner_socket = create_socket_with_bind(config_file, "learners", 1);
    evutil_make_socket_nonblocking(learner_socket);
    subscribe_multicast_group_by_role(config_file, "learners", learner_socket);

    acceptor_sock_fd = create_socket_by_role(config_file, "acceptors", &acceptor_addr);

    struct event *ev_receive_message;
    ev_receive_message = event_new(base, learner_socket, EV_READ|EV_PERSIST, on_receive_message, NULL);
    event_add(ev_receive_message, NULL);

    event_base_dispatch(base);

    event_free(ev_receive_message);
    event_base_free(base);

    return 0;
}