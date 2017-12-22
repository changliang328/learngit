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
#include "bf_tac.h"
#include "bf_config.h"
#define ANDROID_7	(1)

#define DMD_LOG_MAX_LENTH 1024
char dmd_error_buffer[DMD_LOG_MAX_LENTH] = {0};
const char *configName = "/system/etc/bl_chip_params.cfg";

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

int bf_fingerprint_close(hw_device_t *device)
{
    LOGD("%s", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }
	
	disable_spi_ioctl();
	bf_io_exit();

    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;
    pthread_mutex_lock(&dev->lock);	
    destroyWorker(&dev->worker);

	bf_tac_uninit();

    bf_ree_device_destroy(dev->ree_device);
	if(NULL != dev->imagebuf)
		free(dev->imagebuf);
    pthread_mutex_unlock(&dev->lock);
    pthread_mutex_destroy(&dev->lock);

    free(device);
    LOGD("%s end", __func__);
    return 0;
}


uint64_t bf_fingerprint_pre_enroll(struct fingerprint_device __unused *device)
{
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
    status = bf_tac_get_hw_auth_challenge(dev->tac_handle, &(dev->challenge));

    LOGE("pre_enroll produce change= %llu\n", dev->challenge);
    if (status)	
    {
        LOGE("%s failed %i\n", __func__, status);
    }

    //workerSetState(&dev->worker, STATE_NONE);
    pthread_mutex_unlock(&dev->lock);

     return dev->challenge;
}

int bf_fingerprint_enroll(struct fingerprint_device __unused *device,
                              const hw_auth_token_t __unused *hat,
                              uint32_t __unused gid,
                              uint32_t __unused timeout_sec)
{
    int status = 0;
    LOGD("%s\n", __func__);
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
    
	memcpy(&(dev->hat) ,hat,sizeof(hw_auth_token_t));
 	bf_tac_send_enroll_token(&(dev->hat));
	
#if 0
	BF_LOG(" enroll:hat.gid=%llu",gid);
	BF_LOG(" enroll:hat.version =%u",dev->hat.version);
	BF_LOG(" enroll:hat.user_id =%llu",dev->hat.user_id);
	BF_LOG(" enroll:hat.authenticator_id =%llu",dev->hat.authenticator_id);
	BF_LOG(" enroll:hat.authenticator_type = %u",dev->hat.authenticator_type);
	BF_LOG(" enroll:hat.challenge =%llu ",dev->hat.challenge);
	BF_LOG(" enroll:hat.timestamp =%llu   sizeof_token=%d\n",dev->hat.timestamp,sizeof(dev->hat));
#endif
	if(dev->challenge == dev->hat.challenge){
		LOGE("%s  right challenge\n", __func__);
	}
	else{
		LOGE("%s  error challenge,please check challenge in pre_enroll   \n", __func__);
	}
    workerSetState(&dev->worker, STATE_ENROLL);

out:
    pthread_mutex_unlock(&dev->lock);
    return status;
}


int bf_fingerprint_post_enroll(struct fingerprint_device __unused *device)
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
    status = bf_tac_get_hw_auth_challenge(dev->tac_handle, &dev->challenge);
    if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
        status = -EIO;
    }

    //workerSetState(&dev->worker, STATE_NONE);
    pthread_mutex_unlock(&dev->lock);

    return status;
}


uint64_t bf_get_authenticator_id(struct fingerprint_device __unused *device)
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
	dev->authenticator_id = id;
    LOGD("%s id %" PRIu64 "\n", __func__, id);
    //LOGD("%s id %x\n", __func__, id);

    workerSetState(&dev->worker, STATE_NONE);
    pthread_mutex_unlock(&dev->lock);

    return id;
}

int bf_fingerprint_cancel(struct fingerprint_device __unused *device)
{
    LOGD("%s", __func__);

    if ( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }
	#if  defined(ANDROID_7) 
	{
		fingerprint_msg_t msg = {0};
		msg.type = FINGERPRINT_ERROR;		
		msg.data.error = FINGERPRINT_ERROR_CANCELED;
		device->notify(&msg);
	}
	#endif
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;
    pthread_mutex_lock(&dev->lock);
    workerSetState(&dev->worker, STATE_CANCEL);
    pthread_mutex_unlock(&dev->lock);
    return 0;
}

int bf_fingerprint_remove(struct fingerprint_device __unused *device,
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
        //goto out;
    }

    int found = 0; // true if at least one template was found

    if (indices_count <= 0)
    {
        status = -EINVAL;
        LOGE("%s get indices count error:%d\n", __func__, indices_count);
        //goto out;
    }

    status = bf_tac_get_indices(dev->tac_handle, indices, &indices_count);

    if (status)
    {
        LOGE("%s get indices count error:%d\n", __func__, indices_count);
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                 "%s: bf_tac_get_indices failed, status=%d, indices_count=%d\n", __func__, status, indices_count);
        status = -EIO;
        //goto out;
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
            
	 	#if  defined(ANDROID_7)
			fingerprint_msg_t msg = {0};
			msg.type = FINGERPRINT_ERROR;		
			msg.data.error = FINGERPRINT_ERROR_CANCELED;
			device->notify(&msg);
		#endif
        }
    }

    // TODO: fs db not actually updated untill this returns ok.
    status = bf_tac_store_template_db(dev->tac_handle);

    if (status)
    {
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                 "%s: bf_tac_store_template_db failed, status=%d\n", __func__, status);
        status = -EIO;
        //goto out;
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
            
	 	#if  defined(ANDROID_7)
			fingerprint_msg_t msg = {0};
			msg.type = FINGERPRINT_ERROR;		
			msg.data.error = FINGERPRINT_ERROR_CANCELED;
			device->notify(&msg);
		#endif
        }
    }

out:
    workerSetState(&dev->worker, STATE_NONE);

    pthread_mutex_unlock(&dev->lock);

    if (status == -EIO)
    {
    }

    if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
        return -1;
    }

    return 0;
}

int bf_fingerprint_set_active_group(struct fingerprint_device __unused *device,
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

    status = bf_tac_load_user_db(work_path, pathlen + 1,gid);
    free(work_path);

    if (status == (int)BF_TA_ERROR_DB_GLOBAL_DB_FILE_LOST
        || status == (int)BF_TA_ERROR_DB_USER_DB_FILE_LOST
        || status == (int)BF_TA_ERROR_DB_SINGLE_TEMPLATE_FILE_LOST)
    {
        LOGE("%s: bf_tac_load_user_db db file lost; 0x%x", __func__, status);
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

int bf_fingerprint_authenticate(struct fingerprint_device __unused *device,
                                    uint64_t __unused operation_id, __unused uint32_t gid)
{
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) device;
    int status = 0;

    LOGD("%s dev->operation_id=%llu\n", __func__,dev->operation_id);

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

    dev->operation_id= operation_id;

    workerSetState(&dev->worker, STATE_AUTHENTICATE);

out:
    pthread_mutex_unlock(&dev->lock);
    return status;
}
int bf_set_notify(struct fingerprint_device *device,
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


int bf_fingerprint_enumerate(struct fingerprint_device *device)

{
    uint32_t indices[BF_MAX_FINGER]={0};
    uint32_t indices_count = 0;
	int status = 0;
	uint32_t i = 0;
    fingerprint_msg_t msg = {0, {0}};
    LOGD("%s\n", __func__);

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

void bf_fingerprint_notify(const fingerprint_msg_t *msg)
{
	BF_LOG("msg");
}

int bf_fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device)
{
    int status = 0;
    char* filename = NULL;
    void * config = NULL;
    LOGD("%s\n", __func__);
    *device = NULL;
    bf_ca_app_data_t fdatainfo;
    bf_fingerprint_hal_device_t* dev =
        malloc(sizeof(bf_fingerprint_hal_device_t));
	if (!dev)
	{ return -ENOMEM; }
	memset(dev, 0, sizeof(bf_fingerprint_hal_device_t));
	
    if ( NULL == module || NULL == device )
    {
		BF_LOG("+++");
		dev->device.common.tag = HARDWARE_DEVICE_TAG;
		dev->device.common.version = FINGERPRINT_MODULE_API_VERSION_2_0;
		dev->device.common.module = (struct hw_module_t*) module;
		dev->device.common.close = bf_fingerprint_close;
		dev->device.enroll = bf_fingerprint_enroll;
		dev->device.cancel = bf_fingerprint_cancel;
		dev->device.remove = bf_fingerprint_remove;
		dev->device.set_notify = bf_set_notify;
		dev->device.notify = bf_fingerprint_notify;
		dev->device.authenticate = bf_fingerprint_authenticate;
		dev->device.pre_enroll = bf_fingerprint_pre_enroll;
		dev->device.enumerate = bf_fingerprint_enumerate;
		dev->device.get_authenticator_id = bf_get_authenticator_id;
		dev->device.set_active_group = bf_fingerprint_set_active_group;
		dev->device.post_enroll = bf_fingerprint_post_enroll;
    }
	else{
		BF_LOG("+++");

	}
	*device = (hw_device_t*) dev;
    dev->wait_finger_up = 0;
    dev->required_samples = 15;

    dev->ree_device = bf_ree_device_new();
    if (!dev->ree_device) {
        snprintf(dmd_error_buffer, sizeof(dmd_error_buffer) - 1,
                "%s: bf_ree_device_new failed\n", __func__);
		BF_LOG("errno=%d",errno);
        goto err;
    }
	bf_io_init();
	enable_spi_ioctl();
	//imagebuf for debug use
	dev->imagebuf = malloc(DEBUG_IMGBUF_SIZE);

	if(!access(configName, F_OK | R_OK))
	{
		config = bf_load_config(configName);
	}else
	{
		LOGE("<%s>access config file error:%s. Default parameter will set.",
			__func__, strerror(errno));
	}
	//tac init
	status = bf_tac_init(config);
	LOGD("init status %d", status);
	if(status < 0)
	{
		LOGE("<%s>bf_tac_init fail.",__func__);
		goto err1;
	}
	
	if(config != NULL)
	{
		free(config);
		config = NULL;
	}
	bf_get_finger_data(&fdatainfo);
	dev->height = fdatainfo.height;
	dev->width = fdatainfo.width;
	
	clear_pollflag_ioctl();
	//load_config_from file ,debug option,navi func,
    dev->navState = false;

    pthread_mutex_init(&dev->lock, NULL);

    initWorker(dev);
    mmi_info_init(dev);
    initGetScreenStateWorker(dev);

    LOGI("fingeprint module open ok!");

    return 0;
	
err1:
	
	if(config != NULL)
	{
		free(config);
		config = NULL;
	}
	
	if(NULL != dev->imagebuf)
		free(dev->imagebuf);
	
	disable_spi_ioctl();
	bf_io_exit();
	
err:
    bf_ree_device_destroy(dev->ree_device);

    free(dev);
    LOGE("%s failed %i\n", __func__, status);
    return -1;
}                            

