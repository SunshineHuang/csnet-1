inc = -I../../libcsnet/
lib = -L../../libcsnet/
cflags = -std=c99 -O3 -Wall -g

target = client ping-pong
objs = client.o ping-pong.o
all: $(target)

client: client.o
	gcc $(cflags) -o $@ $^ $(inc) $(lib) -lpthread -lcsnet

ping-pong: ping-pong.o
	gcc $(cflags) -o $@ $^ $(inc) $(lib) -lpthread -lcsnet

%.o: %.c
	gcc $(cflags) -c $< $(inc)

clean:
	rm -f *.o
	rm -f $(target)
