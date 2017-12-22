TARGET = libfp_ta.a

SRC_C =  $(TADIR)/core/bf_algo.c \
	 $(TADIR)/core/bf_core.c \
	 $(TADIR)/core/bf_image_info.c \
	 $(TADIR)/core/bf_image_process.c \
	 $(TADIR)/core/bf_crc.c \
	 $(TADIR)/core/bf_template.c  \
	 $(TADIR)/chips/bl_chips.c \
	 $(TADIR)/chips/bl_spi_common.c \
	 ../ta_plat.c \
	 ../../bf_tee_platform_api.c \
	 src/fp_ta_command_router.c \
 	 #$(TADIR)/core/bf_bmp.c \
	 

MY_CFLAGS = -D_TEE_DEBUG_ -DISEE25_TEE -DBUILD_TEE \
	-I$(BFROOTDIR)/include \
	-Iinc \
	$(CFLAGS_SYS_BASE) \
    	$(CFLAGS_PF_CRYPTO) \
   	$(CFLAGS_PF_FP) \
    	$(CFLAGS_PF_KM) \
    	$(CFLAGS_PF_SPI) \
    	$(CFLAGS_PF_TIME) \
    	$(CFLAGS_PF_TS) \

MY_LDFLAGS = $(LDFLAGS_PF_CRYPTO) \
	$(LDFLAGS_PF_FP) \
	$(LDFLAGS_PF_KM) \
	$(LDFLAGS_PF_SPI) \
	$(LDFLAGS_PF_TIME) \
	$(LDFLAGS_PF_TS) 

include $(UT_SDK_HOME)/build/lib.mk
