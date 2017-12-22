ifeq ($O,)
PWD := $(shell pwd)
export O=$(PWD)/out
endif

export CROSS_COMPILE=arm-none-eabi-
## modify BINARY to your own uuid
#export BINARY = e13010e0-2ae1-11e5-896a-0002a5d5c51b
export BINARY = e13010e0-2ae1-11e5-896a-0002a5b5c5d5
#MODULE_NAME = fingerprint_ta
## nodify LIBNAME to your lib name
#export LIBNAME = libtest

## add self defined libs
libdirs +=$(TA_DEV_KIT_DIR)/lib32
libnames+= math utils rsee utee mpa  strcpy

libdirs+=$(PWD)/lib
libnames+=BMF_ei_x2 xuFPAlg  fp_algo  

# Prevent use of LDFLAGS from the environment. For example, yocto exports
# LDFLAGS that are suitable for the client applications, not for TAs
LDFLAGS=

## unmask the following line to ignore warning errors
NOWERROR=y

include $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk

.PHONY: all
all:


clean: clean-out

clean-out:
	rm -fr $O
	rm -fr ../core  ../chips
	rm -f .bf_tee_platform_api.o.cmd
