dev_dir := $(dir $(lastword $(MAKEFILE_LIST)))
cc = gcc-5.1
CFLAGS = -std=c99 -Wall -O3 -fno-omit-frame-pointer -Wpointer-arith -pipe \
         -D_GNU_SOURCE -D_BSD_SOURCE -D_POSIX_SOURCE -D_REENTRANT -DOPEN_PRINT -fPIC -g \
         -I$(dev_dir)libcsnet/ \
         -I$(dev_dir)thirdparty/include \
         -L$(dev_dir)libcsnet \
         -L$(dev_dir)thirdparty/lib
LIBS = -lpthread -ldl -lcsnet
