include ../../public.mk
VPATH = ./: ../../src/
target = slist dlist
all: $(target)

slist: test_slist.o slist.o
	$(cc) $(cflags) -o $@ $^

test_slist.o: test_slist.c
	$(cc) $(cflags) -c $^ $(inc_dir)

slist.o: slist.c
	$(cc) $(cflags) -c $^ $(inc_dir)


dlist: test_dlist.o dlist.o
	$(cc) $(cflags) -o $@ $^

test_dlist.o: test_dlist.c
	$(cc) $(cflags) -c $^ $(inc_dir)

dlist.o: dlist.c
	$(cc) $(cflags) -c $^ $(inc_dir)

clean:
	rm -f *.o
	rm -f $(target)
