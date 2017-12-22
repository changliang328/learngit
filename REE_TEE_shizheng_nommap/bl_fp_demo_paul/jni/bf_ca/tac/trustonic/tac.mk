LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libTeeClient
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libTeeClient.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libbf_tac

LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_SRC_FILES := ../bf_tac_tee.c tee_adapter.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../include 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include 
LOCAL_LDLIBS :=-llog

LOCAL_SHARED_LIBRARIES := libutils \
	                      libcutils \
	                      libbf_core \
	                      libTeeClient \
	                      liblog 

LOCAL_CERTIFICATE := platform
include $(BUILD_SHARED_LIBRARY)
