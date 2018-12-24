#!/usr/bin/env bash
cc -o client.o client.c communication_layer.c /usr/local/lib/libevent.a
cc -o proposer.o proposer.c communication_layer.c /usr/local/lib/libevent.a
cc -o acceptor.o acceptor.c communication_layer.c /usr/local/lib/libevent.a
cc -o learner.o learner.c communication_layer.c /usr/local/lib/libevent.a
