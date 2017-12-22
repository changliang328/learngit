LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libtrusty
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libtrusty.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libbf_tac

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := ../bf_tac_tee.c tee_adapter.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../include 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include 
LOCAL_LDLIBS :=-llog

LOCAL_SHARED_LIBRARIES := libutils \
	                      libcutils \
	                      libbf_core \
	                      libtrusty \
	                      liblog 

LOCAL_CERTIFICATE := platform
include $(BUILD_SHARED_LIBRARY)
