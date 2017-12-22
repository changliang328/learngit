
TOOLCHAIN_PREFIX ?= arm-eabi-
LKROOT     ?= source/external/lk
TOPDIR     := $(shell pwd)
MAKEINC    := $(LKROOT) source/lk/trusty
BUILDROOT  ?= ./build

export TOPDIR
export TOOLCHAIN_PREFIX
export BUILDROOT

$(MAKECMDGOALS) _top:
	make -C $(TOPDIR) -f $(LKROOT)/make/oneshot.mk $(addprefix -I, $(MAKEINC)) $(MAKECMDGOALS)
