LOCAL_PATH := $(call my-dir)

TAC_NAME := 

tac_variant := $(filter ree beanpod qsee isee rsee trustkernel trustonic trusty sprtrusty isee25,$(TAC_NAME))
include $(CLEAR_VARS)
ifeq (,$(tac_variant)) #empty default is ree
TAC_NAME := ree
include $(LOCAL_PATH)/ree/tac.mk

else #TAC_NAME is not empty
include $(LOCAL_PATH)/$(TAC_NAME)/tac.mk
endif
