LOCAL_PATH := $(call my-dir)
BFROOTDIR := $(LOCAL_PATH)/../..

include $(CLEAR_VARS)
LOCAL_MODULE := libbf_hal

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(LOCAL_PATH)/bf_hal.c $(LOCAL_PATH)/bf_fingerprint.c $(LOCAL_PATH)/bf_device.c $(LOCAL_PATH)/cJSON.c $(LOCAL_PATH)/bf_config.c 

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += $(BFROOTDIR)/include 
LOCAL_LDLIBS :=-llog

LOCAL_SHARED_LIBRARIES := libutils \
                          libcutils \
                          liblog \
                          libbf_tac 

LOCAL_CERTIFICATE := platform
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fingerprint.default 
LOCAL_MODULE_FILENAME := fingerprint.default
LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += $(BFROOTDIR)/include 

LOCAL_SRC_FILES := $(LOCAL_PATH)/fingerprint.c 
					
LOCAL_SHARED_LIBRARIES := libbf_hal libbf_tac  liblog libutils libhardware libcutils  
LOCAL_LDLIBS += -llog

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := blestech.fingerprint.default 
LOCAL_MODULE_FILENAME :=blestech.fingerprint.default
LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += $(BFROOTDIR)/include 

LOCAL_SRC_FILES := $(LOCAL_PATH)/fingerprint.c 
					
LOCAL_SHARED_LIBRARIES := libbf_hal libbf_tac  liblog libutils libhardware libcutils  
LOCAL_LDLIBS += -llog

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_CFLAGS += -DBLCOMPATIBLE
include $(BUILD_SHARED_LIBRARY)


