
CURDIR :=.
TADIR  := $(CURDIR)/../../..
BFROOTDIR := $(CURDIR)/../../../..

cppflags-$(CFG_TA_FLOAT_SUPPORT) += -DCFG_TA_FLOAT_SUPPORT=1

#INCLUDE
global-incdirs-y += ../inc
global-incdirs-y += $(BFROOTDIR)/include

cflags-y += -Wno-float-equal  


#SRC
srcs-y +=$(TADIR)/core/bf_algo.c 
srcs-y +=$(TADIR)/core/bf_core.c 
srcs-y +=$(TADIR)/core/bf_image_info.c 
srcs-y +=$(TADIR)/core/bf_image_process.c 
srcs-y +=$(TADIR)/core/bf_crc.c 
srcs-y +=$(TADIR)/core/bf_template.c  
srcs-y +=$(TADIR)/chips/bl_chips.c 
srcs-y +=$(TADIR)/chips/bl_spi_common.c 
srcs-y +=../ta_plat.c 
srcs-y +=../../bf_tee_platform_api.c 
srcs-y +=fp_ta_command_router.c 
srcs-y +=../bf_libc.c
#srcs-y +=$(TADIR)/core/bf_bmp.c 

 

#DEFINE
cflags-y += -DTA_ROUTER_DEBUG  -DBUILD_TEE -DRSEE_TEE 

