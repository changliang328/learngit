################################################################################
#
# <t-sdk Sample Crypto Catalog Trusted Application
#
################################################################################

CURDIR :=.
TADIR  := $(CURDIR)/../..
BFROOTDIR := $(TADIR)/..

# output binary name without path or extension
OUTPUT_NAME := fp_ta
GP_ENTRYPOINTS := Y

#-------------------------------------------------------------------------------
# t-base-convert parameters, see manual for details
#-------------------------------------------------------------------------------
TA_UUID := b9a09f5805815453a0dbe4ef2fda3a6d
TRUSTLET_MEMTYPE := 2
TA_SERVICE_TYPE := SYS
TA_KEYFILE := pairVendorTltSig.pem #pairUUIDKeyFile.pem                      
TA_INSTANCES := 10
#Debug/Release
MODE:=Release
TOOLCHAIN:=GNU


#-------------------------------------------------------------------------------

TBASE_API_LEVEL :=7
HEAP_SIZE_INIT := 2097152 #4M
HEAP_SIZE_MAX := 4194304  #4M

ifeq ($(MODE), Release)
TA_FLAGS := 0
else
TA_FLAGS := 4
endif
#-------------------------------------------------------------------------------
# Files and include paths - Add your files here
#-------------------------------------------------------------------------------
#DEFINE
CC_OPTS2 += -DBUILD_TEE -DTRUSTONIC_TEE 

### Add include path here
INCLUDE_DIRS += include \
                $(BFROOTDIR)/include \


### Add source code files for C compiler here
SRC_C +=$(TADIR)/core/bf_algo.c 
SRC_C +=$(TADIR)/core/bf_core.c 
SRC_C +=$(TADIR)/core/bf_image_info.c 
SRC_C +=$(TADIR)/core/bf_image_process.c 
SRC_C +=$(TADIR)/core/bf_crc.c 
SRC_C +=$(TADIR)/core/bf_template.c  
SRC_C +=$(TADIR)/chips/bl_chips.c 
SRC_C +=$(TADIR)/chips/bl_spi_common.c 
SRC_C +=src/ta_plat.c 
SRC_C +=../bf_tee_platform_api.c 
SRC_C +=src/fp_ta_command_router.c 
SRC_C +=src/bf_libc.c
#SRC_C +=$(TADIR)/core/bf_bmp.c 



### Add source code files for C++ compiler here
SRC_CPP += \


### Add source code files for assembler here
SRC_S += # nothing

CUSTOMER_DRIVER_LIBS += \
	libs/libfp_algo.a \
        libs/libxuFPAlg.a \
        libs/libBMF_ei_x2.a \
	libs/drspi.lib \
	libs/get_hmac_key.lib \
	libs/drutils.lib \
	libs/drsec.lib \


#-------------------------------------------------------------------------------
# use generic make file
TRUSTED_APP_DIR ?= Locals/Code
TASDK_DIR_SRC ?= $(TASDK_DIR)

include $(TASDK_DIR)/trusted_application.mk
