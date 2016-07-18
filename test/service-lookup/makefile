inc = -I../../libcsnet/
lib = -L../../libcsnet/
cflags = -std=c99 -O3 -Wall -g

target = register_client get_items_client
objs = register_client.o get_items_client.o
all: $(target)

register_client: register_client.o
	gcc $(cflags) -o $@ $^ $(inc) $(lib) -lpthread -lcsnet

get_items_client: get_items_client.o
	gcc $(cflags) -o $@ $^ $(inc) $(lib) -lpthread -lcsnet

%.o: %.c
	gcc $(cflags) -c $< $(inc)

clean:
	rm -f *.o
	rm -f $(target)
