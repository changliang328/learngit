/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "FingerprintHal"

#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <cutils/log.h>
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include "bf_fingerprint.h"
static int fingerprint_close(hw_device_t *dev)
{
	return bf_fingerprint_close(dev);
}


static uint64_t fingerprint_pre_enroll(struct fingerprint_device __unused *dev) {

    return bf_fingerprint_pre_enroll(dev);
}

static int fingerprint_enroll(struct fingerprint_device __unused *dev,
                                const hw_auth_token_t __unused *hat,
                                uint32_t __unused gid,
                                uint32_t __unused timeout_sec) {
    return bf_fingerprint_enroll(dev, hat, gid, timeout_sec);
}

static int fingerprint_post_enroll(struct fingerprint_device __unused *dev){
	return bf_fingerprint_post_enroll(dev);
}

static uint64_t fingerprint_get_auth_id(struct fingerprint_device __unused *dev) {
    return bf_get_authenticator_id(dev);
}

static int fingerprint_cancel(struct fingerprint_device __unused *dev) {
    return bf_fingerprint_cancel(dev);
}

static int fingerprint_remove(struct fingerprint_device __unused *dev,
                                uint32_t __unused gid, uint32_t __unused fid) {
    return bf_fingerprint_remove(dev, gid, fid);
}

static int fingerprint_set_active_group(struct fingerprint_device __unused *dev,
                                        uint32_t __unused gid, const char __unused *store_path) {
    return bf_fingerprint_set_active_group(dev, gid, store_path);
}

static int fingerprint_authenticate(struct fingerprint_device __unused *dev,
                                    uint64_t __unused operation_id, __unused uint32_t gid) {
    return bf_fingerprint_authenticate(dev, operation_id, gid);
}

static int set_notify_callback(struct fingerprint_device *dev,
                                fingerprint_notify_t notify) {

    return bf_set_notify(dev, notify);
}

static int fingerprint_enumerate(struct fingerprint_device *dev)
{
	return bf_fingerprint_enumerate(dev);
}

static int fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device)
{
	ALOGE("finger_open++\n");
	int ret = 0;
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

	ret = bf_fingerprint_open(module, id, device);
	if(ret < 0)
		return ret;

    fingerprint_device_t *dev = (fingerprint_device_t *)*device;

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = FINGERPRINT_MODULE_API_VERSION_2_0;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = fingerprint_close;

    dev->pre_enroll = fingerprint_pre_enroll;
    dev->enroll = fingerprint_enroll;
    dev->get_authenticator_id = fingerprint_get_auth_id;
    dev->cancel = fingerprint_cancel;
    dev->remove = fingerprint_remove;
    dev->set_active_group = fingerprint_set_active_group;
    dev->authenticate = fingerprint_authenticate;
    dev->set_notify = set_notify_callback;
    dev->enumerate = fingerprint_enumerate;
    dev->post_enroll = fingerprint_post_enroll;

    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_2_0,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        #ifndef BLCOMPATIBLE
        .id                 = FINGERPRINT_HARDWARE_MODULE_ID,
        #else
        .id                 = "blestech.fingerprint",
        #endif
        .name               = "BetterLife Fingerprint Hal",
        .author             = "BetterLife AE Team",
        .methods            = &fingerprint_module_methods,
    },
};
