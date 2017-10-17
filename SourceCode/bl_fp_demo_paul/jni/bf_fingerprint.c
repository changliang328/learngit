#include <errno.h>
#include <malloc.h>
#include <string.h>

#include <hardware/hardware.h>
#include <hardware/fingerprint.h>

#include <inttypes.h>

#include "bf_hal.h"
#include "bf_log.h"
#include <cutils/log.h>
#include "bf_lib.h"
#include "bf_types.h"

#define DMD_LOG_MAX_LENTH 1024
char dmd_error_buffer[DMD_LOG_MAX_LENTH] = {0};
#define QC_AUTH_NONCE 32
#define FP_DB_FINGERPRINTS_IN_SET   5

#define  BF_TAC_OK                       (0)
#define  BF_TAC_ERROR_GENERAL            (1)
#define  BF_TAC_ERROR_MEMORY             (2)
#define  BF_TAC_ERROR_PARAMETER          (3)
#define  BF_TAC_ERROR_HANDLE             (4)
#define  BF_TAC_ERROR_RANGE              (5)
#define  BF_TAC_ERROR_TIMEOUT            (6)
#define  BF_TAC_ERROR_STATE              (7)
#define  BF_TAC_ERROR_APP_NOT_FOUND      (8)
#define  BF_TAC_ERROR_NO_RESPONSE        (9)
#define  BF_TAC_ERROR_TEEC_INIT_FAIL     (10)


#define DMD_FINGERPRINT_UNLOCK_SUCC_NO              (912000000)
#define DMD_FINGERPRINT_UNLOCK_FAIL_NO              (912000001)
#define DMD_FINGERPRINT_ENROLL_NO                   (912000002)
#define DMD_FINGERPRINT_UNLOCK_STATS_NO             (912000003)
#define DSM_FINGERPRINT_WAIT_FOR_FINGER_ERROR_NO    (912001000)
#define DSM_FINGERPRINT_CAPTURE_IMAGE_ERROR_NO      (912001001)
#define DSM_FINGERPRINT_IDENTIFY_ERROR_NO           (912001002)
#define DSM_FINGERPRINT_TEST_DEADPIXELS_ERROR_NO    (912001003)
#define DSM_FINGERPRINT_ENROLL_ERROR_NO             (912001004)
#define DSM_FINGERPRINT_REMOVE_TEMPLATE_ERROR_NO    (912001005)
#define DSM_FINGERPRINT_ENUMERATE_ERROR_NO          (912001006)
#define DSM_FINGERPRINT_MODULE_OPEN_ERROR_NO        (912001007)
#define DSM_FINGERPRINT_PROBE_FAIL_ERROR_NO         (912001008)
#define DSM_FINGERPRINT_DIFF_DEADPIXELS_ERROR_NO    (912001009)
#define DSM_FINGERPRINT_MANY_DEADPIXELS_ERROR_NO    (912001010)
#define DSM_FINGERPRINT_DB_FILE_LOST_ERROR_NO       (912001011)

static int bf_fingerprint_close(hw_device_t *device)
{
    char str[100] = {0};

    LOGD("%s", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }

    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;

    pthread_mutex_lock(&dev->lock);
/*
    if (dev->hal_ex && dev->hal_ex->tac_ops.hal_ex_close)
    {
        dev->hal_ex->tac_ops.hal_ex_close(dev->tac_handle);
    }

    snprintf(str, 100, "%d", MSG_EXIT);
    send_to_socket_server(str, SOCKET_ADDRESS_NATIVE);
*/
    destroyWorker(&dev->worker);

    //bf_tac_close(dev->tac_handle);
    //bf_tac_deinit(dev->tac_handle);

    bf_ree_device_destroy(dev->ree_device);
    //fingerprint_close();

    pthread_mutex_unlock(&dev->lock);
    pthread_mutex_destroy(&dev->lock);

    free(device);
    device = NULL;
    LOGD("%s end", __func__);
    return 0;
}


static uint64_t bf_fingerprint_pre_enroll(struct fingerprint_device __unused *device)
{
    uint64_t challenge = 0;
    int status = 0;
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;

    LOGD("%s\n", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return 0;
    }

    pthread_mutex_lock(&dev->lock);
    //workerSetState(&dev->worker, STATE_IDLE);

	//modify by paul
    status = bf_tac_get_hw_auth_challenge(dev->tac_handle, &challenge);
	dev->challenge = challenge;
	
    if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
        challenge = 0;
    }

    //workerSetState(&dev->worker, STATE_NONE);
    pthread_mutex_unlock(&dev->lock);

    return challenge;
}

static int bf_fingerprint_enroll(struct fingerprint_device __unused *device,
                              const hw_auth_token_t __unused *hat,
                              uint32_t __unused gid,
                              uint32_t __unused timeout_sec)
{
    int status = 0;
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;

    if ( NULL == device || NULL == hat)
    {
        LOGE("%s return, reason: device or hat is NULL timeout_sec = %d", __func__, timeout_sec);
        return -1;
    }

    pthread_mutex_lock(&dev->lock);

    if (!device->notify)
    {
        LOGE("%s failed notify not set\n", __func__);
        status = -1;
        goto out;
    }

    workerSetState(&dev->worker, STATE_IDLE);

    if (gid != dev->current_gid)
    {
        LOGE("%s finger.gid != current_gid\n", __func__);
        status = -1;
        goto out;
    }

    dev->hat = *hat;

    workerSetState(&dev->worker, STATE_ENROLL);

out:
    pthread_mutex_unlock(&dev->lock);
    return status;
}


static int bf_fingerprint_post_enroll(struct fingerprint_device __unused *device)
{
    uint64_t challenge = 0;
    int status = 0;
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;

    LOGD("%s\n", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: dev is NULL", __func__);
        return -EINVAL;
    }

    pthread_mutex_lock(&dev->lock);
    //workerSetState(&dev->worker, STATE_IDLE);
	
	//modify by paul
    status = bf_tac_get_hw_auth_challenge(dev->tac_handle, &challenge);
	dev->challenge = 0;
    if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
        status = -EIO;
    }

    //workerSetState(&dev->worker, STATE_NONE);
    pthread_mutex_unlock(&dev->lock);

    return status;
}


static uint64_t bf_get_authenticator_id(struct fingerprint_device __unused *device)
{
    uint64_t id = 0;
    int status = 0;
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;

    LOGD("%s\n", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return 0;
    }

    pthread_mutex_lock(&dev->lock);
    workerSetState(&dev->worker, STATE_IDLE);

    status = bf_tac_get_template_db_id(dev->tac_handle, &id);

    if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
        id = 0;
    }

    LOGD("%s id %" PRIu64 "\n", __func__, id);
    //LOGD("%s id %x\n", __func__, id);

    workerSetState(&dev->worker, STATE_NONE);
    pthread_mutex_unlock(&dev->lock);

    return id;
}

static int bf_fingerprint_cancel(struct fingerprint_device __unused *device)
{
    LOGD("%s", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }

    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;
    pthread_mutex_lock(&dev->lock);
    workerSetState(&dev->worker, STATE_CANCEL);
    pthread_mutex_unlock(&dev->lock);
    return 0;
}

static int bf_fingerprint_remove(struct fingerprint_device __unused *device,
                              uint32_t __unused gid, uint32_t __unused fid)
{
    int status = 0;
    uint32_t indices_count = 0;
    uint32_t indices[BF_MAX_FINGER]={0};
    uint32_t i = 0;
    fingerprint_msg_t msg = {0, {0}};
    LOGD("%s", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }
	BF_LOG("fingerprintd remove %d",fid);
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;
    pthread_mutex_lock(&dev->lock);
    workerSetState(&dev->worker, STATE_IDLE);

    if (gid != dev->current_gid)
    {
        LOGD("%s gid != current_gid, nothing to remove\n", __func__);
        goto out;
    }

    status = bf_tac_get_template_count(dev->tac_handle, &indices_count);
    if (status)
    {
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                 "%s: bf_tac_get_template_count failed, status=%d,indices_count=%d\n", __func__, status, indices_count);
        status = -EIO;
        goto out;
    }

    int found = 0; // true if at least one template was found

    if (indices_count <= 0)
    {
        status = -EINVAL;
        LOGE("%s get indices count error:%d\n", __func__, indices_count);
        goto out;
    }

    status = bf_tac_get_indices(dev->tac_handle, indices, &indices_count);

    if (status)
    {
        LOGE("%s get indices count error:%d\n", __func__, indices_count);
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                 "%s: bf_tac_get_indices failed, status=%d, indices_count=%d\n", __func__, status, indices_count);
        status = -EIO;
        goto out;
    }

    for (i = 0; i < indices_count; i++)
    {

        uint32_t template_id;
        status = bf_tac_get_template_id_from_index(dev->tac_handle, indices[i], &template_id);

        if (status)
        {
            snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                     "%s: bf_tac_get_template_id_from_index failed, status=%d, template_id=%d\n", __func__, status, template_id);
            status = -EIO;
            goto out;
        }

        if (fid == 0 || fid == template_id)
        {
            status = bf_tac_delete_template(dev->tac_handle, indices[i]);

            if (status)
            {
                snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                         "%s: bf_tac_delete_template failed, status=%d, template_id=%d\n", __func__, status, template_id);
                status = -EIO;
                goto out;
            }

            if (device->notify)
            {
                LOGD("notify removed fid=%d gid=%d", template_id, dev->current_gid);
                msg.data.removed.finger.fid = template_id;
                msg.data.removed.finger.gid = dev->current_gid;
                msg.type = FINGERPRINT_TEMPLATE_REMOVED;
                device->notify(&msg);
            }

            found = 1;
        }
    }

    if(found)
    {
        if (device->notify)
        {
            LOGD("notify removed fingerID fid=0 gid=%d", dev->current_gid);
            msg.data.removed.finger.fid = 0;
            msg.data.removed.finger.gid = dev->current_gid;
            msg.type = FINGERPRINT_TEMPLATE_REMOVED;
            device->notify(&msg);
        }
    }

    // TODO: fs db not actually updated untill this returns ok.
    status = bf_tac_store_template_db(dev->tac_handle);

    if (status)
    {
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                 "%s: bf_tac_store_template_db failed, status=%d\n", __func__, status);
        status = -EIO;
        goto out;
    }

    if (!found && fid != 0)
    {
        // Fingerprint not found in the database, notify it was already removed by sending
        // FINGERPRINT_TEMPLATE_REMOVED.
        LOGD("fingerprint not found, notifying removed fid=%d gid=%d", fid, dev->current_gid);

        if (device->notify)
        {
            msg.data.removed.finger.fid = fid;
            msg.data.removed.finger.gid = dev->current_gid;
            msg.type = FINGERPRINT_TEMPLATE_REMOVED;
            device->notify(&msg);
        }
    }

out:
    workerSetState(&dev->worker, STATE_NONE);

    pthread_mutex_unlock(&dev->lock);

    if (status == -EIO)
    {
        //dmd_notify(DSM_FINGERPRINT_REMOVE_TEMPLATE_ERROR_NO, dmd_error_buffer);
    }

    if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
        return -1;
    }

    return 0;
}

static int bf_fingerprint_set_active_group(struct fingerprint_device __unused *device,
                                        uint32_t __unused gid, const char __unused *store_path)
{
    int status = 0;
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;

    LOGD("%s store_path=%s gid=%d\n", __func__, store_path, gid);

    if ( NULL == device || NULL == store_path)
    {
        LOGE("%s return, reason: device or store_path is NULL", __func__);
        return -EINVAL;
    }

    pthread_mutex_lock(&dev->lock);
    workerSetState(&dev->worker, STATE_IDLE);

    uint32_t pathlen = strlen(store_path);
    char* work_path = malloc(pathlen + 1);

    if (NULL == work_path)
    {
        LOGE("Failed to allocated memory for path\n");
        pthread_mutex_unlock(&dev->lock);
        return BF_TAC_ERROR_MEMORY;
    }

    strncpy(work_path, store_path, pathlen);
    work_path[pathlen] = '\0';

    status = bf_tac_load_user_db(dev->tac_handle, work_path, pathlen + 1);
    free(work_path);

    if (status == (int)BF_TA_ERROR_DB_GLOBAL_DB_FILE_LOST
        || status == (int)BF_TA_ERROR_DB_USER_DB_FILE_LOST
        || status == (int)BF_TA_ERROR_DB_SINGLE_TEMPLATE_FILE_LOST)
    {
        LOGE("%s: bf_tac_load_user_db db file lost; 0x%x", __func__, status);
        //dmd_notify(DSM_FINGERPRINT_DB_FILE_LOST_ERROR_NO, "fpc DB file lost error\n");
    }

    if (status != 0)
    {
        LOGE("%s: bf_tac_load_user_db failed with error; %d", __func__, status);
        status = 0; // Ignore load failure. The existing (empty) db will remain
    }

    status = bf_tac_set_active_fingerprint_set(dev->tac_handle, gid);

    if (status)
    {
        goto out;
    }

    LOGE("%s: beore bf_tac_check_template_version is ok ", __func__);
   //return    -1 uncheck   / 0 check ok    /1  unmatch   /2 error
    status = -1;
    status =bf_tac_check_template_version(dev->tac_handle);
    LOGE("%s: end bf_tac_check_template_version is ok  check result = %d", __func__,status);
    if (status == 1){//unmatch auto delete
        int tmp_status = 0;
        pthread_mutex_unlock(&dev->lock);
        tmp_status = bf_fingerprint_remove(device, gid, 0);
        pthread_mutex_lock(&dev->lock);
        if (tmp_status){
            dev->check_need_reenroll_finger = 2; //check error
        }
        else {
            dev->check_need_reenroll_finger = 1;//need reEnroll
        }
    }
    else {
        dev->check_need_reenroll_finger = status;//-1 uncheck   /0 check ok /2 check error
    }
/*
    if ((NULL != dev) && (NULL != dev->hal_ex) && (NULL != dev->hal_ex->tac_ops.data_convert))
    {
        dev->hal_ex->tac_ops.data_convert(dev->tac_handle);
    }
*/
    dev->current_gid = gid;
out:

    if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
        status = -1;
    }

    workerSetState(&dev->worker, STATE_NONE);
    pthread_mutex_unlock(&dev->lock);

    return status;
}

static int bf_fingerprint_authenticate(struct fingerprint_device __unused *device,
                                    uint64_t __unused operation_id, __unused uint32_t gid)
{
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;
    int status = 0;

    LOGD("%s", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }

    pthread_mutex_lock(&dev->lock);

    if (!device->notify)
    {
        LOGE("%s failed notify not set\n", __func__);
        status = -1;
        goto out;
    }

    workerSetState(&dev->worker, STATE_IDLE);

    if (gid != dev->current_gid)
    {
        LOGE("%s finger.gid != current_gid\n", __func__);
        status = -1;
        goto out;
    }

    dev->challenge = operation_id;
    dev->nonce = NULL;

    workerSetState(&dev->worker, STATE_AUTHENTICATE);

out:
    pthread_mutex_unlock(&dev->lock);
    return status;
}
static int bf_set_notify(struct fingerprint_device *device,
                               fingerprint_notify_t notify)
{
    LOGD("%s", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }

    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;
    pthread_mutex_lock(&dev->lock);
    workerSetState(&dev->worker, STATE_IDLE);
    device->notify = notify;
    workerSetState(&dev->worker, STATE_NONE);
    pthread_mutex_unlock(&dev->lock);
    return 0;
}


static int bf_fingerprint_enumerate(struct fingerprint_device *device)

{
    uint32_t indices[BF_MAX_FINGER]={0};
    uint32_t indices_count = 0;
	int status = 0;
	uint32_t i = 0;
    fingerprint_msg_t msg = {0, {0}};
    LOGD("%s", __func__);

    if (NULL == device)
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }

    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;

    pthread_mutex_lock(&dev->lock);
    workerSetState(&dev->worker, STATE_IDLE);


    status = bf_tac_get_template_count(dev->tac_handle, &indices_count);

    if (status)
    {
        status = -EIO;
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                "%s: bf_tac_get_template_count failed\n", __func__);
        LOGE("%s get indices count error:%d\n", __func__, indices_count);
        goto out;
    }

    if (indices_count > FP_DB_FINGERPRINTS_IN_SET)
    {
        status = -EINVAL;
        LOGE("%s get indices count error:%d\n", __func__, indices_count);
        goto out;
    }

    if(indices_count == 0 && device->notify)
    {
        LOGD("indices_count == 0, notify enumerate fid=%d gid=%d remaining_templates=%d\n", 0, dev->current_gid, 0);
        msg.data.enumerated.finger.fid = 0;
        msg.data.enumerated.finger.gid = dev->current_gid;
        msg.data.enumerated.remaining_templates = 0;
        msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
        device->notify(&msg);
        goto out;
    }

    status = bf_tac_get_indices(dev->tac_handle, indices, &indices_count);

    if (status)
    {
        status = -EIO;
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                "%s: bf_tac_get_indices failed\n", __func__);
        goto out;
    }

    for (i = 0; i < indices_count; ++i)
    {
        uint32_t template_id;
        status = bf_tac_get_template_id_from_index(dev->tac_handle, indices[i], &template_id);

        if (status)
        {
            status = -EIO;
            snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                    "%s: bf_tac_get_template_id_from_index failed\n", __func__);
            goto out;
        }

        if (device->notify)
        {
            LOGD("notify enumerate fid=%d gid=%d remaining_templates=%d\n", template_id, dev->current_gid, indices_count - i - 1);
            msg.data.enumerated.finger.fid = template_id;
            msg.data.enumerated.finger.gid = dev->current_gid;
            msg.data.enumerated.remaining_templates = indices_count - i - 1;
            msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
            device->notify(&msg);
        }
    }

out:
    workerSetState(&dev->worker, STATE_NONE);

    pthread_mutex_unlock(&dev->lock);

    if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
        //dmd_notify(DSM_FINGERPRINT_ENUMERATE_ERROR_NO, dmd_error_buffer);
        return -1;
    }

    return 0;
}


static int bf_fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device)
{
    int status = 0;
    char* filename = NULL;
    uint32_t len = 0;
    uint32_t sensor_id = 0xFF;
    //uint32_t product_id = 0;
    uint32_t result = 0;
    uint32_t deadpixels_count = 0;
    char nav_status[4] = {'\0'};
    LOGD("%s", __func__);

    if ( NULL == module || NULL == device )
    {
        LOGE("%s return, reason: module or device is NULL name:%s", __func__, id);
        return -EINVAL;
    }

    *device = NULL;

    bf_fingerprint_hal_device_t* dev =
        malloc(sizeof(bf_fingerprint_hal_device_t));

    if (!dev)
    { return -ENOMEM; }

    memset(dev, 0, sizeof(bf_fingerprint_hal_device_t));

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version = FINGERPRINT_MODULE_API_VERSION_2_0;
    dev->device.common.module = (struct hw_module_t*) module;
    dev->device.common.close = bf_fingerprint_close;
    dev->device.enroll = bf_fingerprint_enroll;
    dev->device.cancel = bf_fingerprint_cancel;
    dev->device.remove = bf_fingerprint_remove;
    dev->device.set_notify = bf_set_notify;
    dev->device.notify = NULL;
    dev->device.authenticate = bf_fingerprint_authenticate;
    dev->device.pre_enroll = bf_fingerprint_pre_enroll;
    dev->device.enumerate = bf_fingerprint_enumerate;
    dev->device.get_authenticator_id = bf_get_authenticator_id;
    dev->device.set_active_group = bf_fingerprint_set_active_group;
    dev->device.post_enroll = bf_fingerprint_post_enroll;
    //dev->device.reset_lockout = bf_reset_lockout;
    dev->wait_finger_up = 1;
    //g_fido_dev.authenticate = fido_authenticate;
    //g_fido_dev.set_authenticate_cb = fido_set_authenticate_cb;
    //dev->fido = &g_fido_dev;
    //dev->hal_ex = get_hw_fp_hal_ex_impl();
    dev->need_liveness_authentication = 0;
    dev->required_samples = 15; /* default is 6 */

    dev->ree_device = bf_ree_device_new();
    if (!dev->ree_device) {
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                "%s: bf_ree_device_new failed\n", __func__);
        goto err;
    }

	//imagebuf for debug use
	dev->imagebuf = malloc(DEBUG_IMGBUF_SIZE);

	//tac init
	bf_tac_init(dev);
	//load_config_from file ,debug option,navi func,
    dev->navState = true;
	//
    pthread_mutex_init(&dev->lock, NULL);

    //   -1 uncheck   / 0 check ok    /1  unmatch   /2 error
    dev->check_need_reenroll_finger= -1;
    //LOGE("default  dev->check_need_reenroll_finger result = %d",dev->check_need_reenroll_finger);

    initWorker(dev);

    *device = (hw_device_t*) dev;

 	//create test thread
    //if ((pthread_create(&server_socket_thread, NULL, run_socket_server, dev)) != 0)

    LOGI("fingeprint module open ok!");

	//IC selftest, dead pixcel

	//get software version,algo version,

    //low_power_deep_sleep(dev);
    //workerSetState(&dev->worker, STATE_NAVIGATION);
    //shouldCreateDir(STATS_TMP_FILE);
    //shouldCreateDir(STATS_ALL_SPLASH2_FILE);
    //init_stats_datas();
    //gettimeofday(&updateFingerTemplateTimeLimit, NULL); /*begin DTS2017011805546 baidabin wx373933 20170207 end*/
    return 0;

err_open:
    //bf_tac_close(dev->tac_handle);

err_init:
    //bf_tac_deinit(dev->tac_handle);

err:
    bf_ree_device_destroy(dev->ree_device);

    free(dev);
    LOGE("%s failed %i\n", __func__, status);
    //dmd_notify(DSM_FINGERPRINT_MODULE_OPEN_ERROR_NO, dmd_error_buffer);
    return -1;
}                            

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = bf_fingerprint_open,
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
