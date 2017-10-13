LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CERTIFICATE := platform
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
LOCAL_LDLIBS    := -lm -llog
LOCAL_MODULE := libnavgation

LOCAL_SRC_FILES :=  utils.c \
					core.c \
					nav_algo_2.c
				
				
LOCAL_SHARED_LIBRARIES := libutils \
                          libcutils 


include $(BUILD_SHARED_LIBRARY)
