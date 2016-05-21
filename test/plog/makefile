include ../../public.mk
link = -lpthread
VPATH = ./: ../../src/

target = test_plog test_stress_plog test_blocking_write

all: $(target)

test_stress_plog: test_stress_plog.o condition.o plog.o cpu_binding.o
	$(cc) $(cflags) -o $@ $^ $(link)

test_plog: test_plog.o condition.o plog.o cpu_binding.o
	$(cc) $(cflags) -o $@ $^ $(link)

test_stress_plog.o: test_stress_plog.c
	$(cc) $(cflags) -c $^ $(inc_dir)

test_plog.o: test_plog.c
	$(cc) $(cflags) -c $^ $(inc_dir)

condition.o: condition.c
	$(cc) $(cflags) -c $^ $(inc_dir)

plog.o: plog.c
	$(cc) $(cflags) -c $^ $(inc_dir)

cpu_binding.o: cpu_binding.c
	$(cc) $(cflags) -c $^ $(inc_dir)

test_blocking_write: test_blocking_write.c
	$(cc) $(cflags) -o $@ $< $(inc_dir) $(link)

clean:
	rm -f *.o
	rm -f $(target)
	rm -f *.log
