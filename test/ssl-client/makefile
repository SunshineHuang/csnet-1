inc_dir = -I../../libcsnet/ 
lib_dir = -L../../libcsnet/
cflags = -std=c99 -O3 -Wall -g

VPATH = ./: ../../src

target = ssl_client

all: $(target)

ssl_client: ssl_client.o
	gcc $(cflags) -o $@ $^ $(lib_dir) -lcsnet -lpthread -lssl -lcrypto

%.o: %.c
	gcc $(cflags) -c $^ $(inc_dir)

clean:
	rm -f *.o
	rm -f $(target)
