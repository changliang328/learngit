export CHIPS_ROOT:=../../../../chips
export CORE_ROOT:=../../../../core

global-incdirs-y +=include

#subdirs +=  
srcs-y += ta_entry.c

srcs-y +=../../ta_plat.c\
	 ../../../bf_tee_platform_api.c

srcs-y +=$(CHIPS_ROOT)/bl_spi_common.c \
	 $(CHIPS_ROOT)/bl_chips.c 

srcs-y +=$(CORE_ROOT)/bf_algo.c \
	 $(CORE_ROOT)/bf_image_info.c \
	 $(CORE_ROOT)/bf_image_process.c \
	 $(CORE_ROOT)/bf_crc.c \
	 $(CORE_ROOT)/bf_template.c\
	 $(CORE_ROOT)/bf_core.c \


	# $(CORE_ROOT)/bf_bmp.c \  



#define flag  
ta-cflags += -DBUILD_TEE -DTK_TEE -I$(CHIPS_ROOT)/../../include

#libs
ta-ldflags += -L./libs  -lfp_algo -lBMF_ei_x2 -lxuFPAlg 


