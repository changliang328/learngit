LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libbtlfp

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := bl29xxfinger.c fingerprintinvoke.c
LOCAL_MULTILIB := both 
LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)

LOCAL_SHARED_LIBRARIES := libutils \
                          libcutils \
                          libBtlFpHal \
                          libBtlAlgo

LOCAL_CERTIFICATE := platform
include $(BUILD_SHARED_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libnavgation
#LOCAL_SRC_FILES := ../lib64/libnavgation.so
#LOCAL_SRC_FILES := ./libs/armeabi-v7a/libBtlAlgo.so
#LOCAL_MULTILIB := both
#LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#LOCAL_MODULE_SUFFIX := .so
#include $(PREBUILT_SHARED_LIBRARY)

#$(shell cp -f $(TARGET_OUT)/lib/libcdfingerprint.so $(LOCAL_PATH)/../algolibs/lib/libcdfingerprint.so)
#$(shell cp -f $(TARGET_OUT)/lib64/libcdfingerprint.so $(LOCAL_PATH)/../algolibs/lib64/libcdfingerprint.so)


