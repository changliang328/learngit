#使用前，请定义TDS_SDK_ROOT和TDS_WORKSPACE两个环境变量。
#TDS_SDK_ROOT是tds的sdk的路径。
#TDS_WORKSPACE是its开发的路径。

include $(TDS_SDK_ROOT)/build/inner_depend.mk

TARGET = libbf_core.a

SRC_C =  bf_algo.c \
		 bf_bmp.c \
		 bf_core.c \
		 bf_image_info.c \
		 bf_image_process.c \
		 bf_crc.c \
		 bf_template.c 

#-DLOG_LV_INFO 
MY_CFLAGS =  -I$(BFROOTDIR)/include \
    -I$(INCLUDE_PRIVATE_CONTRIB)/lib_common \
    $(INCLUDE_PLATFORM_FP)\
    $(INCLUDE_PLATFORM_KM) \
    -D BUILD_TEE\
    -D ISEE_TEE
#MY_LDFLAGS = -Llibs -lfpc
	
include $(TDS_SDK_ROOT)/build/lib.mk
