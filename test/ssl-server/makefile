inc_dir = -I../../src/ -I/usr/local/Cellar/openssl/1.0.1j/include -I../../third-part-lib/include/libxml2
lib_dir = -L../../src/ -L/usr/local/Cellar/openssl/1.0.1j/lib -L../../third-part-lib/lib
cflags = -std=c99 -O3 -Wall -g

VPATH = ./: ../../src

objs = epoller.o cs_ssl.o socket_api.o \
       condition.o

target = ssl_server

all: $(target)

ssl_server: ssl_server.o $(objs)
	gcc $(cflags) -o $@ $^ $(lib_dir) -lpthread -lssl -lcrypto

%.o: %.c
	gcc $(cflags) -c $^ $(inc_dir)

clean:
	rm -f *.o
	rm -f $(target)
