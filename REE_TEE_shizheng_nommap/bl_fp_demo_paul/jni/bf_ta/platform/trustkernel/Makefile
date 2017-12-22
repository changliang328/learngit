O := $(CURDIR)/out

include $(CURDIR)/public/version.mk
ifeq (${CROSS_COMPILE},)
TA_CROSS_COMPILE := arm-linux-gnueabihf-
else
TA_CROSS_COMPILE := ${CROSS_COMPILE}
endif

ifeq ($(TKCORE_SDK),)
$(error TKCORE_SDK is not defined)
endif

ifneq ($V,1)
q := @
else
q :=
endif

TA_DIRS := \
	proj_ta/default\


.PHONY: all clean  

all:
ifneq (,)
	
endif
	$(q)$(foreach dir,$(TA_DIRS), \
		$(MAKE) -C $(dir) O=$(O)/$(dir) TA_VERSION_SRC=$(VERSION_SRC) TKCORE_APPS_COMMITID=$(TKCORE_APPS_COMMITID) \
		CROSS_COMPILE="$(TA_CROSS_COMPILE)" \
		q=$(q) &&) true

clean:
	$(q)$(foreach dir,$(TA_DIRS), $(MAKE) -C $(dir) O=$(O)/$(dir) TA_VERSION_SRC=$(VERSION_SRC) q=$(q) $@ &&) true
 


