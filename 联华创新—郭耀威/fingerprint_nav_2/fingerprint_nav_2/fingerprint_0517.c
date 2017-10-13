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
#include <hardware/hw_auth_token.h>
#include <hardware/fingerprint.h>
//#include <hardware/bl29xxfinger.h>
#include "btlfp.h"

extern int32_t  Fp_InitEnroll();
extern int32_t  Fp_Enroll     (fingerprint_device_t __unused *dev,
                        const hw_auth_token_t __unused * token,
                        uint32_t __unused gid,
                        uint32_t __unused timeout_sec);
extern int32_t  Fp_Cancel   (fingerprint_device_t __unused *dev);
extern int32_t  Fp_PostEnroll(fingerprint_device_t __unused *dev);
extern int32_t  Fp_Remove (fingerprint_device_t __unused *dev,uint32_t __unused gid, uint32_t __unused fid);
extern int64_t  Fp_GetAuthId(fingerprint_device_t __unused *dev);
extern uint64_t Fp_PreEnroll(fingerprint_device_t __unused *dev);
extern int32_t  Fp_Authenticate(fingerprint_device_t __unused *dev,uint64_t __unused operation_id, __unused uint32_t gid);
extern int32_t  Fp_SetActiveGroup(fingerprint_device_t __unused *dev,uint32_t __unused gid, const char __unused *store_path);
extern int32_t  Fp_InitLock();
#ifdef BTL_ANDROID_N
extern int32_t  Fp_Enumerate(fingerprint_device_t *dev);
#else
extern int32_t Fp_Enumerate(fingerprint_device_t __unused *dev, fingerprint_finger_id_t __unused *results,
                     uint32_t *max_size);
#endif
static int fingerprint_close(hw_device_t *dev)
{
    if (dev) {
        free(dev);
        return 0;
    } else {
        return -1;
    }
}


static uint64_t fingerprint_pre_enroll(struct fingerprint_device __unused *dev)
{

    ALOGD("fingerprint_pre_enroll");

    uint64_t token = Fp_PreEnroll(dev);

    if (token != 0) return token;

    return -1;
}

static int fingerprint_enroll(struct fingerprint_device __unused *dev,
                              const hw_auth_token_t __unused *hat,
                              uint32_t __unused gid,
                              uint32_t __unused timeout_sec)
{

    ALOGD("fingerprint_enroll");

    if (Fp_Enroll(dev,hat,gid,timeout_sec) != 0) return -1;

    return 0;
}


static int fingerprint_post_enroll(struct fingerprint_device __unused *dev)
{

    ALOGD("fingerprint_post_enroll");

    if (Fp_PostEnroll(dev) != 0) return -1;

    return 0;
}


static uint64_t fingerprint_get_auth_id(struct fingerprint_device __unused *dev)
{

    return Fp_GetAuthId(dev);
}

static int fingerprint_cancel(struct fingerprint_device __unused *dev)
{

    if (Fp_Cancel(dev)  != 0)return -1;

    return 0;
}

static int fingerprint_remove(struct fingerprint_device __unused *dev,
                              uint32_t __unused gid, uint32_t __unused fid)
{

    if (Fp_Remove(dev,gid,fid) != 0) return -1;

    return 0;
}

static int fingerprint_set_active_group(struct fingerprint_device __unused *dev,
                                        uint32_t __unused gid, const char __unused *store_path)
{
    Fp_SetActiveGroup(dev,gid,store_path);
    return 0;
}

static int fingerprint_authenticate(struct fingerprint_device __unused *dev,
                                    uint64_t __unused operation_id, __unused uint32_t gid)
{

    if (Fp_Authenticate(dev,operation_id, gid) < 0) return -1;

    return 0;
}

static int set_notify_callback(struct fingerprint_device *dev,
                               fingerprint_notify_t notify)
{
    /* Decorate with locks */
    dev->notify = notify;


    if(notify == NULL) return -1;

    return 0;
}

#ifdef BTL_ANDROID_N
static int fingerprint_enumerate(struct fingerprint_device *dev)

{
    Fp_Enumerate(dev);
    return 0;
}
#else
static int fingerprint_enumerate(struct fingerprint_device __unused  *dev, fingerprint_finger_id_t __unused  *results,
                                 uint32_t __unused  *max_size)
{
    Fp_Enumerate(dev,results,max_size);
    return 0;
}
#endif
static int fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device)
{
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    fingerprint_device_t *dev = malloc(sizeof(fingerprint_device_t));
    memset(dev, 0, sizeof(fingerprint_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = FINGERPRINT_MODULE_API_VERSION_2_0;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = fingerprint_close;

    dev->pre_enroll = fingerprint_pre_enroll;
    dev->enroll = fingerprint_enroll;
    dev->post_enroll = fingerprint_post_enroll;
    dev->get_authenticator_id = fingerprint_get_auth_id;
    dev->cancel = fingerprint_cancel;
    dev->remove = fingerprint_remove;
    dev->set_active_group = fingerprint_set_active_group;
    dev->authenticate = fingerprint_authenticate;
    dev->set_notify = set_notify_callback;
    dev->enumerate = fingerprint_enumerate;
    dev->notify = NULL;

    *device = (hw_device_t*) dev;

    Fp_InitLock();
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
        .id                 = "fingerprint",
        .name               = "BetterLife Fingerprint Hal",
        .author             = "BetterLife AE Team",
        .methods            = &fingerprint_module_methods,
    },
};
