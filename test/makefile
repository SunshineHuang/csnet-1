subdirs = bounded-queue hashtable list midd-server mutex-queue pack plog recv-buffer timer

target:
	for dir in $(subdirs); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(subdirs); do \
		$(MAKE) -C $$dir clean; \
	done
