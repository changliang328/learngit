LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := jnibtlfp 

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)de/REE_TEE_FingerprintHal/bl_fp_demo_paul
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include 
#LOCAL_SRC_FILES := $(LOCAL_PATH)/btl_jni.c 
#LOCAL_SRC_FILES := $(LOCAL_PATH)/bf_jni_workstate.c
LOCAL_SRC_FILES := $(LOCAL_PATH)/bf_jni_openhal.c				
LOCAL_SHARED_LIBRARIES := libbf_hal libbf_tac liblog libutils libcutils  
LOCAL_LDLIBS += -llog

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
include $(BUILD_SHARED_LIBRARY)
