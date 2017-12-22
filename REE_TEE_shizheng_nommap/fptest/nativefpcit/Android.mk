LOCAL_PATH:= $(call my-dir)
include 	$(CLEAR_VARS)
# Modulename should match apk name to be installed
LOCAL_PACKAGE_NAME := nativefpcit
LOCAL_MODULE_TAGS:= optional 
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res
LOCAL_CERTIFICATE := platform
include $(BUILD_PACKAGE) 

