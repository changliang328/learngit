LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += \
	src/com/btlfinger/service/aidl/IFpsFingerClient.aidl \
	src/com/btlfinger/service/aidl/IFpsFingerManager.aidl 

LOCAL_RESOURCE_DIR += $(addprefix $(LOCAL_PATH)/, res)
LOCAL_PACKAGE_NAME := BtlFingerprint

LOCAL_MULTILIB := both 

LOCAL_PROGUARD_ENABLED := full
LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
