SUBDIRS := fp_ta fp_server
CURDIR := $(shell pwd)
TADIR := $(CURDIR)/../..
BFROOTDIR = $(CURDIR)/../../..
#COREDIRS = $(TADIR)/core $(TADIR)/chips 
export TADIR
export BFROOTDIR
.PHONY: all clean
all:
	@for entry in $(SUBDIRS); \
	do \
	if test $$entry != "include" -a $$entry != "data" -a $$entry != "fp_lib"; then \
	   	if test -d $(CURDIR)/$$entry; then \
	      	cd $$entry; \
	      	make -w || exit "$$?";\
	      	cd ..; \
	   	fi; \
	fi; \
	done

clean:
	@for entry in $(SUBDIRS); \
	do \
	if test -d $(CURDIR)/$$entry; then \
	cd $$entry; \
	make -w clean; \
	cd ../; \
	fi; \
done

