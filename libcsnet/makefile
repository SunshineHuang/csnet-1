include ../public.mk
srcs = $(wildcard *.c)
cobjs = $(srcs:.c=.o)
objdir = .objs
objs = $(addprefix $(objdir)/, $(cobjs))

$(objdir):
	@mkdir -p $@

target = libcsnet.a

all: $(target)

$(target): $(objs)
	ar rc $@ $^

$(objdir)/%.o: %.c | $(objdir)
	$(cc) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(objdir)
	rm -f $(target)
