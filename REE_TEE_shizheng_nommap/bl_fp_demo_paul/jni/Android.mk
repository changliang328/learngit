LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libbl_fp_alg
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libbl_fp_alg.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libxuFPAlg
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libxuFPAlg.so
include $(PREBUILT_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))

