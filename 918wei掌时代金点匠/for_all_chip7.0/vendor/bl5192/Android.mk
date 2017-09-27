LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fingerprint.btl
LOCAL_SRC_FILES_64 := ./libs/arm64-v8a/fingerprint.default.so
LOCAL_SRC_FILES_32 := ./libs/armeabi-v7a/fingerprint.default.so
LOCAL_MULTILIB := both
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_MODULE := libBtlFpHal
LOCAL_SRC_FILES_64 := ./libs/arm64-v8a/libBtlFpHal.so
LOCAL_SRC_FILES_32 := ./libs/armeabi-v7a/libBtlFpHal.so
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_MODULE := libBtlAlgo
LOCAL_SRC_FILES_64 := ./libs/arm64-v8a/libBtlAlgo.so
LOCAL_SRC_FILES_32 := ./libs/armeabi-v7a/libBtlAlgo.so
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libxuFPAlg
LOCAL_SRC_FILES_64 := ./libs/arm64-v8a/libxuFPAlg.so
LOCAL_SRC_FILES_32 := ./libs/armeabi-v7a/libxuFPAlg.so
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)




