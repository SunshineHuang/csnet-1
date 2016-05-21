include ../../public.mk
VPATH = ./: ../../src/

target = test test1

all: $(target)

test: test.o hashtable.o config.o
	$(cc) $(cflags) -o $@ $^

test1: test1.o int_hash.o
	$(cc) $(cflags) -o $@ $^

test.o: test.c
	$(cc) $(cflags) -c $^ $(inc_dir)

test1.o: test1.c
	$(cc) $(cflags) -c $^ $(inc_dir)

hashtable.o: hashtable.c
	$(cc) $(cflags) -c $^ $(inc_dir)

int_hash.o: int_hash.c
	$(cc) $(cflags) -c $^ $(inc_dir)

config.o: config.c
	$(cc) $(cflags) -c $^ $(inc_dir)

clean:
	rm -f *.o
	rm -f $(target)
