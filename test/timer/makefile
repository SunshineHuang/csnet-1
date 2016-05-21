include ../../public.mk
VPATH = ./: ../../src/
objs = int_hash.o timer.o
target = test
all: $(target)

test: test.o $(objs)
	$(cc) $(cflags) -o $@ $^

test.o: test.c
	$(cc) $(cflags) -c $^ $(inc_dir)

%.o: %.c
	$(cc) $(cflags) -c $^ $(inc_dir)

clean:
	rm -f *.o
	rm -f $(target)
