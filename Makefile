CFLAGS+= -O2 -Wall -Wextra
PACKAGES+= libevent
OBJS= client.o proposer.o acceptor.o learner.o communication_layer.o

PKG_CONFIG_CFLAGS!= pkg-config --cflags ${PACKAGES}
PKG_CONFIG_LIBS!= pkg-config --libs ${PACKAGES}
CFLAGS+= ${PKG_CONFIG_CFLAGS}
LDLIBS+= ${PKG_CONFIG_LIBS}
COMPILE= ${CC} ${CPPFLAGS} ${CFLAGS} -c
LINK= ${CC} ${CPPFLAGS} ${CFLAGS} ${LDFLAGS}

all: client proposer acceptor learner

clean:
	rm -f client proposer acceptor learner ${OBJS}

client: client.o communication_layer.o
	${LINK} client.o communication_layer.o ${LDLIBS} -o $@

proposer: proposer.o communication_layer.o
	${LINK} proposer.o communication_layer.o ${LDLIBS} -o $@

acceptor: acceptor.o communication_layer.o
	${LINK} acceptor.o communication_layer.o ${LDLIBS} -o $@

learner: learner.o communication_layer.o
	${LINK} learner.o communication_layer.o ${LDLIBS} -o $@

${OBJS}: communication_layer.h utils.h

.SUFFIXES:

.SUFFIXES: .c .o

.c.o:
	${COMPILE} $<

.PHONY: all clean
