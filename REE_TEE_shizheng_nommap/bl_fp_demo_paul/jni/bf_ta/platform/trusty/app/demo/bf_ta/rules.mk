# Copyright (C) 2015 The Android Open Source Project
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
#

CURDIR :=.
TADIR  := $(CURDIR)/../..
BFROOTDIR := $(CURDIR)/../../..


LOCAL_DIR := $(GET_LOCAL_DIR)
$(warning compiling bf_ta enter =============)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/manifest.c \
	$(LOCAL_DIR)/trusty_tademo.cpp \
	$(LOCAL_DIR)/tademo_ipc.cpp 

MODULE_SRCS += \
	$(TADIR)/core/bf_algo.c \
	$(TADIR)/core/bf_core.c  \
	$(TADIR)/core/bf_image_info.c  \
	$(TADIR)/core/bf_image_process.c  \
	$(TADIR)/core/bf_crc.c   \
	$(TADIR)/core/bf_template.c    \
	$(TADIR)/chips/bl_chips.c   \
	$(TADIR)/chips/bl_spi_common.c   \
	$(TADIR)/platform/bf_tee_platform_api.c  \
	$(TADIR)/platform/trusty/app/demo/bf_ta/ta_plat.c  

IPC := ipc

MODULE_DEPS += \
	app/trusty \
 
MODULE_DEPS_STATIC += \
	libc \
	rng \
	openssl\
	libc-trusty \
	libstdc++-trusty \
	storage \
	keymaster \

MODULE_INCLUDES += \
	$(LOCAL_DIR) \
	$(BFROOTDIR)/include \
	$(LOCAL_DIR)/inc  \
	
 
GLOBAL_OPTFLAGS +=-DBUILD_TEE -DTRUSTY_TEE 


include make/module-user_task.mk
MODULE_DEPS_STATIC += $(LOCAL_DIR)/lib/libfp_algo.a
MODULE_DEPS_STATIC += $(LOCAL_DIR)/lib/libxuFPAlg.a
MODULE_DEPS_STATIC += $(LOCAL_DIR)/lib/libBMF_ei_x2.a

include make/module.mk
$(warning compiling bf_ta leave =============)

