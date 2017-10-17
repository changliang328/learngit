# Copyright (C) 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libBtlAlgo
LOCAL_SRC_FILES := ./libs/$(TARGET_ARCH_ABI)/libBtlAlgo.so
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libxuFPAlg
LOCAL_SRC_FILES := ./libs/$(TARGET_ARCH_ABI)/libxuFPAlg.so
LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
#LOCAL_CFLAGS := -Wall -Wextra -Werror -Wunused
LOCAL_MODULE := libprocess
LOCAL_SRC_FILES := bl_process.c 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include 

LOCAL_MULTILIB := both
LOCAL_LDLIBS :=-llog
LOCAL_SHARED_LIBRARIES := \
	libBtlAlgo \
	libxuFPAlg \
	liblog \
	libutils 
include $(BUILD_SHARED_LIBRARY)


