##### paths #####
INCDIR = ../../src
LIBDIR = ../../src
OBJDIR = .
SRCDIR = .: ../../src

##### misc #####
QUIETLY = 1>nul 2>nul

##### sources, objects and libraries #####
BINNAME = test
BINARY  = $(BINNAME)
SRCDIRS = .
SOURCES = abstraction.c benchmark_queue.c  main.c misc.c test_queue.c queue.c
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(SOURCES)))
SYSLIBS = -lpthread -lc -lm
USRLIBS = 

##### CPU variants #####
UNAME   = $(shell uname -m)
GCCARCH = -march=$(UNAME)

ifeq ($(UNAME),x86_64)
  GCCARCH = -march=core2
endif

##### tools #####
MAKE    = make
MFLAGS  = 

DG      = gcc
DGFLAGS = -MM -std=c99 -I"$(SRCDIR)" -I"$(INCDIR)" 

CC      = gcc
CFBASE  = -Wall -Wno-unknown-pragmas -std=c99 $(GCCARCH) -pthread -c -I"$(SRCDIR)" -I"$(INCDIR)"
CFREL   = -O2 -Wno-strict-aliasing
CFDBG   = -O0 -g

LD      = gcc
LFBASE  = -L"$(LIBDIR)"
LFREL   = -O2 -s
LFDBG   = -O0 -g

##### variants #####
CFLAGS  = $(CFBASE) $(CFDBG)
LFLAGS  = $(LFBASE) $(LFDBG)

ifeq ($(MAKECMDGOALS),rel)
  CFLAGS  = $(CFBASE) $(CFREL)
  LFLAGS  = $(LFBASE) $(LFREL)
endif

##### search paths #####
vpath %.c $(patsubst %,$(SRCDIR)/%:,$(SRCDIRS))

##### implicit rules #####
$(OBJDIR)/%.o : %.c
	$(DG) $(DGFLAGS) $< >$(OBJDIR)/$*.d
	$(CC) $(CFLAGS) -o $@ $<

##### explicit rules #####
$(BINARY) : $(OBJECTS)
	$(LD) -o $(BINARY) $(LFLAGS) $(OBJECTS) $(USRLIBS) $(SYSLIBS)
	chmod +x $(BINARY)

##### phony #####
.PHONY : clean rel dbg

clean : 
	@rm -f $(BINNAME) $(OBJDIR)/*.o $(OBJDIR)/*.d

rel : $(BINARY)
dbg : $(BINARY)

##### dependencies #####
-include $(DEPENDS)

