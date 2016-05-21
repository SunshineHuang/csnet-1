cc = gcc
cflags = -std=c11 -Wall -fno-omit-frame-pointer -Wpointer-arith -pipe -D_REENTRANT -DOPEN_PRINT -fPIC -O2 -g
inc_dir = -I../src/
srcs = $(wildcard *.c)
objs = $(srcs:.c=.o)

target = ds-doubly-linked-list-test \
         ds-singly-linked-list-test \
         ds-stack-test \
	 ds-queue-test \
         cds-stack-test \
	 lfqueue-benchmark \
	 binary-tree-test \
	 rbtree-test \
	 priority-queue-test \
	 hashtable-test

all: $(target)

ds-doubly-linked-list-test: ds-doubly-linked-list-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds

ds-singly-linked-list-test: ds-singly-linked-list-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds

ds-stack-test: ds-stack-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds

ds-queue-test: ds-queue-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds

cds-stack-test: cds-stack-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds -lpthread

lfqueue-benchmark: lfqueue-benchmark.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds -lpthread

binary-tree-test: binary-tree-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds -lpthread

rbtree-test: rbtree-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds -lpthread

priority-queue-test: priority-queue-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds -lpthread

hashtable-test: hashtable-test.o
	$(cc) $(cflags) -o $@ $^ -L../ -lcsds -lpthread

%.o: %.c
	$(cc) $(cflags) -c $< -o $@ $(inc_dir)

test:
	./ds-doubly-linked-list-test
	./ds-singly-linked-list-test
	./ds-stack-test
	./ds-queue-test

.PHONY: clean
clean:
	rm -rf $(objs)
	rm -f $(target)
