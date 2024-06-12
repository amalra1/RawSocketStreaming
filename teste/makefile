server_name=server
client_name=client

server_objs=libServer.o server.o
client_objs=libServer.o client.o

CFLAGS := -Wall -lncurses

all: $(server_name) $(client_name)

$(server_name): $(server_objs)
	gcc -o $(server_name) $(server_objs) $(CFLAGS)

$(client_name): $(client_objs)
	gcc -o $(client_name) $(client_objs) $(CFLAGS)

libServer.o: libServer.h libServer.c
	gcc -c libServer.c $(CFLAGS)

server.o: server.c
	gcc -c server.c $(CFLAGS)

client.o: client.c
	gcc -c client.c $(CFLAGS)

clean:
	rm -f $(server_objs) $(client_objs) *~

purge: clean
	-rm -f $(server_name) $(client_name)
