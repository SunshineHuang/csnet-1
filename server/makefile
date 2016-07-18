subdirs = midd-server edge-server ssl-server service-lookup
 
target:
	for dir in $(subdirs); do \
		$(MAKE) -C $$dir; \
	done
 
clean:
	for dir in $(subdirs); do \
		$(MAKE) -C $$dir clean; \
	done

