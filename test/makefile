subdirs = client  pack  recv-buffer  service-lookup  ssl-client

target:
	for dir in $(subdirs); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(subdirs); do \
		$(MAKE) -C $$dir clean; \
	done
