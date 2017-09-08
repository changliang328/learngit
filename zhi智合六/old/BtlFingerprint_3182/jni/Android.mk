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

include $(CLEAR_VARS)
LOCAL_MODULE := libBtlFpHal
LOCAL_SRC_FILES := btlfp_preprocess.c btlfp_core.c btlfp_bmp.c btlfp_sql.c btlfp_hw.c btlfp_util.c bmp_utils.c btlfp_file.c
LOCAL_SHARED_LIBRARIES := \
		liblog \
		libsqlite \
		libBtlAlgo\
		libxuFPAlg
LOCAL_MODULE_TAGS := optional
#LOCAL_CFLAGS += -Werror
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libbtlfp
LOCAL_SRC_FILES := btlfp_android5.c
LOCAL_SHARED_LIBRARIES := liblog libBtlAlgo libBtlFpHal
LOCAL_MODULE_TAGS := optional
# LOCAL_CFLAGS += -Werror

include $(BUILD_SHARED_LIBRARY)
