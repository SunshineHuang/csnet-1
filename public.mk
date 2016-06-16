dev_dir := $(dir $(lastword $(MAKEFILE_LIST)))
cc = gcc
CFLAGS = -std=c99 -Wall -Wpointer-arith -Wsign-compare -Wno-unused-result \
         -fno-omit-frame-pointer -funroll-loops \
         -fPIC -pipe -g3 -O3 \
         -D_GNU_SOURCE -D_BSD_SOURCE -D_POSIX_SOURCE -D_REENTRANT -DOPEN_PRINT \
         -I$(dev_dir)libcsnet/ \
         -I$(dev_dir)thirdparty/include \
         -L$(dev_dir)libcsnet \
         -L$(dev_dir)thirdparty/lib
LIBS = -lcsnet -lpthread -ldl
