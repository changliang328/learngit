SUBDIRS := fp_ta_lib fp_server 
CURDIR := $(shell pwd)
TADIR := $(CURDIR)/../..
BFROOTDIR = $(CURDIR)/../../..
COREDIRS = $(TADIR)/core $(TADIR)/chips 
DEFINES = -DLOG_LV_TRACE
export TADIR
export BFROOTDIR

.PHONY: all clean $(SUBDIRS) 

all:$(SUBDIRS)
	
$(SUBDIRS):
	@if test $@ != "include" -a $@ != "data"; then \
		if test -d $(CURDIR)/$@; then \
			echo "sky  $@"; \
			cd $@; \
			make -w || exit "$$?"; \
			cd $(CURDIR); \
		fi; \
	fi

$(COREDIRS):
	@if test $@ != "include" -a $@ != "data"; then \
		if test -d $@; then \
			echo "sky $@"; \
			cd $@; \
			make -w || exit "$$?"; \
			cd $(CURDIR); \
		fi; \
	fi
fp_server:fp_ta_lib $(COREDIRS)
clean:
	@for entry in $(SUBDIRS); \
	do \
	if test -d $(CURDIR)/$$entry; then \
	cd $$entry; \
	make -w clean; \
	cd ../; \
	fi; \
	done
	@rm -rf *.o  fp_ta_lib/*.o  ../../chips/obj   ../../core/obj/



