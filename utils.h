#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

struct paxos_value
{
    char paxos_value_val[1024];
};
typedef struct paxos_value paxos_value;

struct paxos_prepare
{
    uint32_t instance_id;
    uint32_t ballot;
};
typedef struct paxos_prepare paxos_prepare;

struct paxos_promise
{
    uint32_t acceptor_id;
    uint32_t instance_id;
    uint32_t ballot;
    uint32_t value_ballot;
    paxos_value value;
};
typedef struct paxos_promise paxos_promise;

struct paxos_accept
{
    uint32_t instance_id;
    uint32_t ballot;
    paxos_value value;
};
typedef struct paxos_accept paxos_accept;

struct paxos_accepted
{
    uint32_t acceptor_id;
    uint32_t instance_id;
    uint32_t ballot;
    uint32_t value_ballot;
    paxos_value value;
};
typedef struct paxos_accepted paxos_accepted;

struct paxos_nack
{
    uint32_t instance_id;
    uint32_t promised_ballot;
};
typedef struct paxos_nack paxos_nack;

struct paxos_client_value
{
    uint32_t instance_id;
    paxos_value value;
};
typedef struct paxos_client_value paxos_client_value;

struct paxos_has_hole
{
    uint32_t instance_id_low;
    uint32_t instance_id_high;
};
typedef struct paxos_has_hole paxos_has_hole;

struct paxos_leader
{
    uint32_t leader_id;
    uint32_t last_instance_id;
};
typedef struct paxos_leader paxos_leader;

enum paxos_message_type
{
    PAXOS_PREPARE,
	PAXOS_PROMISE,
	PAXOS_ACCEPT,
	PAXOS_ACCEPTED,
    PAXOS_NACK,
    PAXOS_CLIENT_VALUE,
    PAXOS_HAS_HOLE,
    PAXOS_LEADER
};
typedef enum paxos_message_type paxos_message_type;

struct paxos_message
{
    paxos_message_type type;
    union
    {
        paxos_prepare prepare;
		paxos_promise promise;
		paxos_accept accept;
		paxos_accepted accepted;
        paxos_nack nack;
        paxos_client_value client_value;
        paxos_has_hole has_holes;
        paxos_leader leader;
    } u;
};
typedef struct paxos_message paxos_message;

#endif

