include ../../../public.mk
CFLAGS += -I$(dev_dir)business/include \
          -L$(dev_dir)src

obj_dir = .objs
local_srcs = $(wildcard *.c)
local_objs = $(local_srcs:.c=.o)
objs = $(addprefix $(obj_dir)/, $(local_objs))

$(obj_dir):
	@mkdir -p $@

target = business_module.so

all: $(target)

$(target): $(objs)
	$(cc) $(CFLAGS) -shared -nostartfiles -o $@ $(objs) $(LIBS)

$(obj_dir)/%.o: %.c | $(obj_dir)
	$(cc) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(obj_dir)
	rm -f $(target)
