LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libbf_core
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(LOCAL_PATH)/bf_algo.c $(LOCAL_PATH)/bf_core.c $(LOCAL_PATH)/bf_image_process.c $(LOCAL_PATH)/bf_image_info.c $(LOCAL_PATH)/bf_crc.c $(LOCAL_PATH)/bf_template.c $(LOCAL_PATH)/bf_bmp.c 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include  
LOCAL_LDLIBS :=-llog
LOCAL_STATIC_LIBRARIES := libblchips  libbf_tee_platform_api
LOCAL_SHARED_LIBRARIES := libutils \
                          libcutils \
                          liblog \
                          libxuFPAlg \
                          libbl_fp_alg

LOCAL_CERTIFICATE := platform
include $(BUILD_STATIC_LIBRARY)
