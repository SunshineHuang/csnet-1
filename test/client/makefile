inc = -I../../libcsnet/
lib = -L../../libcsnet/
cflags = -std=c99 -O3 -Wall -g

target = one-by-one-conn-test  one-conn-test  stress-test
objs = one-by-one-conn-test.o  one-conn-test.o  stress-test.o
all: $(target)

one-by-one-conn-test: one-by-one-conn-test.o
	gcc $(cflags) -o $@ $^ $(inc) $(lib) -lpthread -lcsnet -lssl -lcrypto

one-conn-test: one-conn-test.o
	gcc $(cflags) -o $@ $^ $(inc) $(lib) -lpthread -lcsnet -lssl -lcrypto

stress-test: stress-test.o
	gcc $(cflags) -o $@ $^ $(inc) $(lib) -lpthread -lcsnet -lssl -lcrypto

%.o: %.c
	gcc $(cflags) -c $< $(inc)

clean:
	rm -f *.o
	rm -f $(target)
