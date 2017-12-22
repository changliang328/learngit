LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libbf_tee_platform_api
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(LOCAL_PATH)/bf_tee_platform_api.c $(LOCAL_PATH)/ree/ta_plat.c $(LOCAL_PATH)/ree/ree_ca_ta.c
LOCAL_SRC_FILES += $(LOCAL_PATH)/../../hmac_sha/hmac_sha2.c $(LOCAL_PATH)/../../hmac_sha/sha2.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include  
LOCAL_LDLIBS :=-llog

LOCAL_SHARED_LIBRARIES := libutils \
                          libcutils \
                          liblog 

LOCAL_CERTIFICATE := platform
include $(BUILD_STATIC_LIBRARY)

