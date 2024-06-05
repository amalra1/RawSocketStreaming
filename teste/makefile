nome=server

objs=libServer.o server.o

CFLAGS := -Wall -lncurses

all: $(nome)

$(nome): $(objs)
	gcc -o $(nome) $(objs) $(CFLAGS)

libServer.o: libServer.h libServer.c
	gcc -c libServer.c $(CFLAGS)

server.o: server.c
	gcc -c server.c $(CFLAGS)

clean:
	rm -f $(objs) *~

purge: clean
	-rm -f $(nome)
