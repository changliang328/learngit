ifeq ($(FP_SENSOR_TYPE), )
	FP_SENSOR_TYPE = default
endif

ifeq ($(FPC_LIB_TYPE), )
	FPC_LIB_TYPE = release
endif

TARGET = fp_server
  
SRC_CC = src/fp_main.cc

MY_CFLAGS = -DLOG_LV_INFO \
	-DFP_SENSOR_TYPE=\"$(FP_SENSOR_TYPE)\" \
	-DFPC_LIB_TYPE=\"$(FPC_LIB_TYPE)\" \
	-I../fp_ta/inc \
	$(CFLAGS_SYS_BASE) \
	$(CFLAGS_PF_CRYPTO) \
	$(CFLAGS_PF_FP) \
	$(CFLAGS_PF_KM) \
	$(CFLAGS_PF_SPI) \
	$(CFLAGS_PF_TIME) \
	$(CFLAGS_PF_TS) \

MODE=static
MY_LDFLAGS = $(LDFLAGS_PF_CRYPTO) \
	$(LDFLAGS_SYS_BASE) \
	$(LDFLAGS_PF_FP) \
	$(LDFLAGS_PF_KM) \
	$(LDFLAGS_PF_SPI) \
	$(LDFLAGS_PF_TIME) \
	$(LDFLAGS_PF_TS) \
	-L../fp_ta/obj -lfp_ta \
	-L./lib/  -lfp_algo  -lxuFPAlg  -lBMF_ei_x2  
include $(UT_SDK_HOME)/build/prog.mk


