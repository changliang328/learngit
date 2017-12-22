LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MULTILIB:=64
#LOCAL_CFLAGS := -Wall -Wextra -Werror -Wunused

LOCAL_C_INCLUDES := \
	interface \

LOCAL_SRC_FILES:= \
        main_BtlFingerprintService.cpp \
        BtlFingerprintService.cpp \
        interface/IBtlFingerprintService.cpp \
	interface/IDaemonCallback.cpp \
	interface/IProcessBridge.cpp 
		
LOCAL_SHARED_LIBRARIES := \
			libbinder \
			libc \
			libutils \
			libcutils \
			liblog \
			
	
LOCAL_LDLIBS := -llog 	
	
LOCAL_MODULE:= bf_fingerprintd

include $(BUILD_EXECUTABLE)
