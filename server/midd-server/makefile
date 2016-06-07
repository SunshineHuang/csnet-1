include ../../public.mk
#jemalloc = -DJEMALLOC
ifeq ($(jemalloc), -DJEMALLOC)
    CFLAGS += -I../../../third-part/jemalloc/include \
              -L../../../third-part/jemalloc/lib \
	      $(jemalloc)
    LIBS += -ljemalloc
endif
CFLAGS += -rdynamic
LIBS += -lssl -lcrypto

obj_dir = .objs
local_srcs = $(wildcard *.c)
local_objs = $(local_srcs:.c=.o)
objs = $(addprefix $(obj_dir)/, $(local_objs))

$(obj_dir):
	@mkdir -p $@

target = midd-server

all: $(target)

$(target): $(objs)
	$(cc) $(CFLAGS) -o $@ $(objs) $(LIBS)

$(obj_dir)/%.o: %.c | $(obj_dir)
	$(cc) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(obj_dir)
	rm -f $(target)
