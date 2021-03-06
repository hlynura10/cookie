TEAM = cookie
VERSION = 1
HANDINDIR = /labs/proxylab/handin

CC = gcc
CFLAGS = -Wall -g 
LDFLAGS = -pthread

OBJS = proxy.o csapp.o
TEST = test.o csapp.o

all: proxy

proxy: $(OBJS)

test: $(TEST)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c
	$(CC) $(CFLAGS) -c proxy.c

handin:
	cp proxy.c $(HANDINDIR)/$(TEAM)-$(VERSION)-proxy.c
	chmod 600 $(HANDINDIR)/$(TEAM)-$(VERSION)-proxy.c


clean:
	rm -f *~ *.o proxy core

