#使用前，请定义TDS_SDK_ROOT和TDS_WORKSPACE两个环境变量。
#TDS_SDK_ROOT是tds的sdk的路径。
#TDS_WORKSPACE是its开发的路径。

include $(TDS_SDK_ROOT)/build/inner_depend.mk

TARGET = fp_server
#TARGET_FILE_NAME = fp_server_excute
SRC_CC =  src/fp_main.cc
INCLUDE_FILES = include/fp_server.h
MY_CFLAGS =  \
	-Iinclude \
    -I$(INCLUDE_PRIVATE_CONTRIB)/lib_common \
	$(INCLUDE_PLATFORM_FP) \
	-D BUILD_TEE \
	-D ISEE_TEE

MY_LDFLAGS = -lmsg -ldrivers_spi -Llibs -L../fp_ta_lib/obj -L$(TADIR)/core/obj -L$(TADIR)/chips/obj -lfp_ta $(LIB_PLATFORM_FP) $(LIB_PLATFORM_KM) -lutsem -larm_time  -lbf_core -lfp_algo -lBMF_ei_x2 -lxuFPAlg -lbf_chip
include $(TDS_SDK_ROOT)/build/prog.mk
	


