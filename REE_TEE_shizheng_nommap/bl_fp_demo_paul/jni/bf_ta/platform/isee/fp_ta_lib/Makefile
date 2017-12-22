#使用前，请定义TDS_SDK_ROOT和TDS_WORKSPACE两个环境变量。
#TDS_SDK_ROOT是tds的sdk的路径。
#TDS_WORKSPACE是its开发的路径。

include $(TDS_SDK_ROOT)/build/inner_depend.mk

TARGET = libfp_ta.a

SRC_C =  ../ta_plat.c \
         ../../bf_tee_platform_api.c \
         src/fp_ta_command_router.c
         

#-DLOG_LV_INFO 
MY_CFLAGS =  -Iinclude \
	     -I$(BFROOTDIR)/include \
             -I$(INCLUDE_PRIVATE_CONTRIB)/lib_common \
              $(INCLUDE_PLATFORM_FP)\
              $(INCLUDE_PLATFORM_KM) \
	-D BUILD_TEE \
	-D ISEE_TEE \
#MY_LDFLAGS = -Llibs -lfpc
	
include $(TDS_SDK_ROOT)/build/lib.mk
