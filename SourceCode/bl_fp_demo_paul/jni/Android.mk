LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libbl_fp_alg
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libbl_fp_alg.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libxuFPAlg
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libxuFPAlg.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libjnibtlfp

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := btl_jni.c bf_hal.c bf_device.c bf_algo.c bf_tac.c bf_core.c bf_image_process.c  bf_image_info.c bf_template.c
LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include 
LOCAL_LDLIBS :=-llog

LOCAL_SHARED_LIBRARIES := libutils \
                          libcutils \
                          libxuFPAlg \
                          libbl_fp_alg \
                          liblog \
                          libblchips

LOCAL_CERTIFICATE := platform
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fingerprint.default 

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include 

LOCAL_SRC_FILES := bf_fingerprint.c 
					
LOCAL_SHARED_LIBRARIES := libjnibtlfp liblog libutils libhardware libcutils 
LOCAL_LDLIBS += -llog

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))



