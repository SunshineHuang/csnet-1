subdirs = libcsnet libmodule business server
 
target:
	for dir in $(subdirs); do \
		$(MAKE) -C $$dir; \
	done
 
clean:
	for dir in $(subdirs); do \
		$(MAKE) -C $$dir clean; \
	done

