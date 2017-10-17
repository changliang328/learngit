#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

#include <jni.h>

#include "bf_log.h"
#include <android/log.h>

#include <sys/wait.h>

#include <errno.h>

#include <poll.h>
#include "bf_hal.h"
#include "bf_tac.h"

#include "bf_lib.h"
#include "bf_types.h"
#include <endian.h>

#define FINGERUP_TIME (60)

#define MODE_FG_DT					0x02
struct bf_fingerprint_hal_device_t *g_bf_hal_fp_dev;
int isIdentifyImageFailed = 0;

#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
#endif

static int workerShouldCancel(worker_thread_t* worker)
{
    pthread_mutex_lock(&worker->mutex);
    int should = (worker->request == STATE_IDLE);
    pthread_mutex_unlock(&worker->mutex);
    return should;
}

int32_t do_capture(bf_fingerprint_hal_device_t* dev)
{
    int32_t status = 0;
    fingerprint_msg_t message = {0, {0}};

    if ( NULL == dev )
    {
        LOGE("%s return, reason: dev is NULL", __func__);
        return -EINVAL;
    }
    do
    {
        //status = capture_image(dev);
        message.type = FINGERPRINT_ACQUIRED;

        if (status == -EAGAIN)
        {
            message.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_TOO_FAST;
            dev->device.notify(&message);
#ifdef BF_HAL_DEBUG
            if (!isEnroll)
            {
                cJSONunLockvalue[MAX_CAPTURE + 8]++;
                gUnLockValInt[E912000001_FST_CNT_INT]++;
            }
#endif
            continue;
        }
        else if (status < 0 || status == BF_LIB_FAIL_LOW_QUALITY)
        {
            return status;
        }

        if (workerShouldCancel(&dev->worker))
        {
            dev->wait_finger_up = 0;
            LOGD("operation canceled");
            return -EINTR;
        }

    }
    while (status == -EAGAIN);

    return status;
}

int low_power_deep_sleep(bf_fingerprint_hal_device_t* dev)
{
	
}

int32_t wait_finger_up(bf_fingerprint_hal_device_t* dev,
                              uint32_t timeout_ms, int interruptible)
{
	int ret = 0;
	while(ret != -ETIME)
	{
		BF_LOG("++++");
		bf_tac_fd_mode(FINGER_DETECT_DOWN);
		ret = bf_ree_device_wait_irq_paul_timeout(dev->ree_device, FINGERUP_TIME);
		if(ret < 0)
		{
			if(ret == -ETIME)
				dev->wait_finger_up = 0;
			else
			{//cancel
				if(interruptible)
					break;
			}
		}else if(ret == POLLERR)
		{
			//abnormal irq come ,may be in ESD,  hwreset
		}
	}
	return ret;
}
/*
int32_t wait_finger_up(bf_fingerprint_hal_device_t* dev,
                              uint32_t timeout_ms, int interruptible)

{
    uint32_t data = 0;
    int status = 0;
    struct timeval stop, start, delta;
    uint32_t total_sleep_time = 0;

    gettimeofday(&start, NULL);

#ifdef TAC_TIME_MEASUREMENTS
    timer_start(&timer2);
#endif
    for (;;)
    {
        if (interruptible && workerShouldCancel(&dev->worker))
        {
            status = -EINTR;
            break;
        }

        status = bf_tac_check_finger_lost(dev->tac_handle, &data);

        if (status == BF_LIB_FINGER_LOST)
        {
            LOGD("%s BF_LIB_FINGER_LOST\n", __func__);
            status = 0;
            dev->wait_finger_up = 0;
            break;
        }
        else if (status == BF_LIB_ERROR_SENSOR)
        {
            //snprintf(dmd_error_buffer, sizeof(dmd_error_buffer)-1,"%s bf_tac_check_finger_lost error, status=%d, data=%d\n", __func__, status, data);
            status = -EIO;
            dev->wait_finger_up = 0;  // device error, so cancle this operation
            break;
        }
        else
        {
            usleep(CHECK_FINGER_UP_SLEEP_TIME_MS * USECS_PER_MSEC);

            if (timeout_ms)
            {
                gettimeofday(&stop, NULL);
                timersub(&stop, &start, &delta);
                total_sleep_time =
                    (delta.tv_sec * USECS_PER_SECOND + delta.tv_usec) /
                    USECS_PER_MSEC;

                if (total_sleep_time >= timeout_ms)
                {
                    LOGD("%s timed out after %u\n", __func__, total_sleep_time);
                    status = 0;
                    break;
                }
            }
        }
    }

    if (status == -EINTR)
    {
        LOGE("%s interrupted\n", __func__);
    }
    else if (status)
    {
        LOGE("%s failed %i\n", __func__, status);
    }

#ifdef TAC_TIME_MEASUREMENTS
    time_wait += timer_stop(&timer2);
    LOGD("%s, took %d ms", __func__, timer_stop(&timer2));
#endif
    if((-EIO) == status)
    {
        //dmd_notify(DSM_FINGERPRINT_WAIT_FOR_FINGER_ERROR_NO, dmd_error_buffer);
    }
    return status;
}
*/

int wait_finger_down(bf_fingerprint_hal_device_t* dev, int mode)
{
    int ret = 0;
    BF_LOG("++++");
	bf_tac_fd_mode(FINGER_DETECT_DOWN);
	ret = bf_ree_device_wait_irq_paul(dev->ree_device);
	if(ret < 0)
	{
		//be canceled
	}else if(ret == POLLERR)
	{
		//abnormal irq come ,may be in ESD,  hwreset
	}
	return ret;
}

int32_t do_navigation(bf_fingerprint_hal_device_t* dev, int request)
{

	int ret = 0;
	int value = 0;
    
	bf_tac_fd_mode(FINGER_DETECT_DOWN);
	ret = wait_finger_down(dev, 0);
	if(ret < 0)
	{
		//be canceled
	}else if(ret == POLLERR)
	{
		//abnormal irq come ,may be in ESD,  hwreset
	}
	
	value = bf_tac_get_intStatus(&value);
	BF_LOG("REGA_INTR_STATUS=%x ", value);
	if(value == 2)
	{
		bf_tac_read_frame(dev->imagebuf);
	}

	return ret;
}

void do_identify(bf_fingerprint_hal_device_t* dev)
{
	int ret = 0;
    bf_identify_data_t identify_data = {0};
	fingerprint_msg_t acqu_msg_keep_awake = {0};
	fingerprint_msg_t msg = {0};
    int value = 0;
    int status = 0;
    uint32_t indices_count = 0;
    uint32_t indices[BF_MAX_FINGER];
    uint32_t template_id = 0;

	LOGD("do_identify !");

    status = bf_tac_get_indices(dev->tac_handle, indices, &indices_count);

    if (status)
    {
        LOGE("%s get indices count error:%d\n", __func__, indices_count);
        status = -EIO;
        goto out;
    }

	
	for(;;)
	{
		ret = wait_finger_down(dev, 0);
		if(ret < 0)
		{
			break;//be canceled
		}else if(ret == POLLERR)
		{
			//abnormal irq come ,may be in ESD,  hwreset
		}
	
		//check status if ok
		value = bf_tac_get_intStatus(&value);
		BF_LOG("REGA_INTR_STATUS=%x ", value);
		if(value == 2)
		{
			acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
			acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
			dev->device.notify(&acqu_msg_keep_awake);
			
			msg.type = FINGERPRINT_AUTHENTICATED;
			
			bf_tac_read_frame(dev->imagebuf);
			ret = bf_tac_identify(&identify_data);
			BF_LOG("ret=%d index=%d",ret, identify_data.index);
			if((ret == 0) && (identify_data.index >= 0))
			{
				bf_tac_get_template_id_from_index(dev->tac_handle, indices[identify_data.index], &template_id);
				BF_LOG("template_id=%d index=%d",template_id, identify_data.index);

		        msg.data.authenticated.finger.gid = dev->current_gid;
		        msg.data.authenticated.finger.fid = template_id;
		        msg.data.authenticated.hat.user_id = dev->current_gid;//gFingerprintVar.gid;
		        bf_tac_get_template_db_id(NULL, &msg.data.authenticated.hat.authenticator_id);//gFingerprintVar.token;
		        msg.data.authenticated.hat.authenticator_type = htobe32(HW_AUTH_FINGERPRINT);
		        msg.data.authenticated.hat.challenge = dev->challenge;
				struct timespec ts;
				clock_gettime(CLOCK_MONOTONIC, &ts);

		        msg.data.authenticated.hat.timestamp = htobe64((uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
				BF_LOG("gid=%d fid=%d user_id=%d authenticator_id=%x authenticator_type=%x challenge=%x timestamp=%x", msg.data.authenticated.finger.gid, msg.data.authenticated.finger.fid, msg.data.authenticated.hat.user_id, msg.data.authenticated.hat.authenticator_id, msg.data.authenticated.hat.authenticator_type,msg.data.authenticated.hat.challenge, msg.data.authenticated.hat.timestamp);
		        dev->device.notify(&msg);
		        break;
			}else{
				msg.data.authenticated.finger.gid = dev->current_gid;
                msg.data.authenticated.finger.fid = 0;
                dev->device.notify(&msg);
			}
		}else
		{
			//reinit the chip
		}
		
		ret = wait_finger_up(dev, 0, 1);
		if((ret < 0) && (ret != -ETIME))
		{
			break;//cancel
		}
	}
out:
 	return ;
}

//cancel
void notify_enroll_cancel(bf_fingerprint_hal_device_t* dev)
{
    LOGD("notify_enroll_cancel !");
    fingerprint_msg_t message = {0, {0}};
    message.data.error = FINGERPRINT_ERROR_CANCELED;
    message.type = FINGERPRINT_ERROR;
    dev->device.notify(&message);
}

void notify_authenticate_cancel(bf_fingerprint_hal_device_t* dev)
{
    LOGD("notify_authenticate_cancel !");
    fingerprint_msg_t message = {0, {0}};
    message.data.error = FINGERPRINT_ERROR_CANCELED;
    message.type = FINGERPRINT_ERROR;
    dev->device.notify(&message);
}
void do_cancel(bf_fingerprint_hal_device_t* dev, request_cancel_type c_type)
{
    BF_LOG("c_type=%d",c_type);
    switch(c_type)
    {
    case CANCEL_ENROLL:
        notify_enroll_cancel(dev);
        break;
    case CANCEL_AUTHENTICATE:
        notify_authenticate_cancel(dev);
        break;
    default:
        break;
    }
}

//
void do_enroll(bf_fingerprint_hal_device_t* dev)
{
	int ret = 0;
	int value = 0;
	int lastenroll = 0;
	bf_enroll_data_t enrolldata = {0,0,0,0,0,0};
	
	fingerprint_msg_t acqu_msg_keep_awake = {0};
	fingerprint_msg_t msg = {0};
	
	u32 fid = 0;
	bf_tac_new_fid(&fid);
	
	for(;;)
	{
		ret = wait_finger_down(dev, 0);
		if(ret < 0)
		{
			goto out;//cancel;//be canceled
		}else if(ret == POLLERR)
		{
			//abnormal irq come ,may be in ESD,  hwreset
		}
		
		//check status if ok
		value = bf_tac_get_intStatus(&value);
		BF_LOG("REGA_INTR_STATUS=%x ", value);
		if(value == 2)
		{
			acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
			acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
			dev->device.notify(&acqu_msg_keep_awake);
			bf_tac_read_frame(dev->imagebuf);
			//check image quality is ok
		}else
		{
			//reinit the chip
		}
		
		//enroll one frame
		ret = bf_tac_enroll(&enrolldata);
		if(lastenroll != enrolldata.progress)
		{
			msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
			msg.data.enroll.finger.fid = fid;
			msg.data.enroll.samples_remaining = 15 - enrolldata.progress;

			lastenroll = enrolldata.progress;
			dev->device.notify(&msg);
			if(msg.data.enroll.samples_remaining == 0)
			{	
				break;
			}

		}
		
		ret = wait_finger_up(dev, 0, 1);
		if((ret < 0) && (ret != -ETIME))
		{
			goto out;//cancel
		}
	}
out:
	if((ret < 0) && (ret != -ETIME))
	{
		bf_tac_delete_fid(fid);
	}else
	{
		bf_tac_store_template_db(dev->tac_handle);
	}
	return ;
}

int32_t do_mmi_test(bf_fingerprint_hal_device_t* dev)
{}



request_cancel_type get_cancel_type(worker_state_t last_state)
{
    switch(last_state)
    {
    case STATE_ENROLL:
        return CANCEL_ENROLL;
    case STATE_AUTHENTICATE:
        return CANCEL_AUTHENTICATE;
    default:
        return CANCEL_NONE;
    };
}

int workerSetState(worker_thread_t* worker, worker_state_t state)
{
	BF_LOG("+++");
    pthread_mutex_lock(&worker->mutex);

    request_cancel_type c_type = CANCEL_NONE;
    //get lastrequest type,auth enroll none
    if(state == STATE_CANCEL)
    {
        //c_type = get_cancel_type((worker_state_t)(worker->request));
        c_type = get_cancel_type((worker_state_t)(worker->state));
        if(c_type != CANCEL_NONE)
        {
            LOGD("get_cancel_type should send cancel !");
        }
    }
    	BF_LOG("c_type=%d+++",c_type);
	//current state,if not idle become idle first,than to the target state
    if (worker->state != STATE_IDLE)
    {
        worker->request = STATE_IDLE;

        bf_fingerprint_hal_device_t* hal_dev = NULL;

        /* Get the hal device handle from worker pointer */
        hal_dev = container_of(worker, bf_fingerprint_hal_device_t,
                               worker);
        //cancle the wait_irq
        bf_ree_device_set_cancel(hal_dev->ree_device);
		//wait the workfunc become idle
        pthread_cond_wait(&worker->idle_cond, &worker->mutex);
        //clear the pipe for next cancle use
        bf_ree_device_clear_cancel(hal_dev->ree_device);
    }
	//target state
    if (state != STATE_IDLE)
    {
        worker->request = state;
        if(state == STATE_CANCEL)
        {//if target state cancle ,set the cancle type
            worker->c_type = c_type;
        }
        //wakeup workfunc
        pthread_cond_signal(&worker->task_cond);
    }

    pthread_mutex_unlock(&worker->mutex);

    return 0;
}

void* workerFunction(void* data)
{
    bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) data;
    worker_state_t request = STATE_IDLE;
    request_cancel_type c_type = CANCEL_NONE;
    int ret = 0;

    if ( NULL == data )
    {
        LOGE("%s return, reason: data is NULL", __func__);
        return NULL;
    }    
    
    while(1)
    {
        pthread_mutex_lock(&dev->worker.mutex);
        request = (worker_state_t)(dev->worker.request);
        c_type = dev->worker.c_type;
		//boot up ,first enter,default state depend on the previous stat
		LOGE("request=%d",request);
        if (request == STATE_NONE)
        {

            if (dev->wait_finger_up)
            {
                dev->worker.request = STATE_WAIT_FINGER_UP; // make sure finger up befor other operation
            }
            else if (dev->navState == true)
            {
                dev->worker.request = STATE_NAVIGATION;
            }
            else
            {
                dev->worker.request = STATE_IDLE;
            }

            request = (worker_state_t)(dev->worker.request);
        }

        if (request == STATE_IDLE)
        {
            LOGD("worker idle");
            dev->worker.state = STATE_IDLE;
            pthread_cond_signal(&dev->worker.idle_cond); // set state ok , notify threads continue work
            pthread_cond_wait(&dev->worker.task_cond, &dev->worker.mutex); // now idle, wait for task.
        }

        request = (worker_state_t)(dev->worker.request);
        c_type = dev->worker.c_type;
        dev->worker.state = request;

        dev->worker.request = STATE_NONE;

        pthread_mutex_unlock(&dev->worker.mutex);
        
        //do the target state ,target task
#ifdef TAC_TIME_MEASUREMENTS
        time_wait = 0;
        time_cost = 0;
        time_wait_identify_once = 0;
#endif
        switch (request)
        {
            case STATE_CANCEL:
                LOGD("%s STATE_CANCEL\n", __func__);
                do_cancel(dev, c_type);
                break;
            case STATE_ENROLL:
                LOGD("%s STATE_ENROLL\n", __func__);
#ifdef TAC_TIME_MEASUREMENTS
                timer_start(&timer1);
#endif
                do_enroll(dev);

#ifdef TAC_TIME_MEASUREMENTS
                time_cost = timer_stop(&timer1);
                LOGD("do_enroll took %d ms, total:%d wait:%d", time_cost - time_wait, time_cost, time_wait);
#endif

                break;

            case STATE_AUTHENTICATE:
                LOGD("%s STATE_AUTHENTICATE\n", __func__);
#ifdef TAC_TIME_MEASUREMENTS
                timer_start(&timer1);
#endif
                //do {
                do_identify(dev);
                //} while(isIdentifyImageFailed);

#ifdef TAC_TIME_MEASUREMENTS
                time_cost = timer_stop(&timer1);
                LOGD("do_identify took %d ms, total:%d wait:%d", time_cost - time_wait, time_cost, time_wait);
#endif

                break;


            case STATE_NONE:
                break;

            case STATE_NAVIGATION:
                LOGD("%s STATE_NAVIGATION\n", __func__);
                ret = do_navigation(dev, STATE_NAVIGATION);
                if(0 != ret)
                {
                    LOGD("%s do_navigation exit, ret = %d\n", __func__, ret);
                }
                break;

            case STATE_MMITEST:
                LOGD("%s STATE_MMITEST\n", __func__);
                ret = do_mmi_test(dev);
                if(0 != ret)
                {
                    LOGD("%s do_mmi_test exit, ret = %d\n", __func__, ret);
                }
                break;

            case STATE_WAIT_FINGER_UP:
                LOGD("%s STATE_WAIT_FINGER_UP\n", __func__);
                ret = wait_finger_up(dev, 0, 1); // if cancled here, we will continue wait in authentication operation
                if(0 != ret)
                {
                    LOGD("%s wait_finger_up exit, ret = %d\n", __func__, ret);
                }
                break;

            case STATE_EXIT:
                LOGI("STATE_EXIT");
                pthread_exit(NULL);
                goto out;

            default:
                goto out;
        }
    }
    
out:
    LOGD("worker exit");
    return NULL;
}
int32_t initWorker(bf_fingerprint_hal_device_t* dev)
{
    pthread_mutex_init(&dev->worker.mutex, NULL);
    pthread_cond_init(&dev->worker.task_cond, NULL);
    pthread_cond_init(&dev->worker.idle_cond, NULL);
    dev->worker.request = STATE_NONE;
    dev->worker.state = STATE_NONE;
    return pthread_create(&dev->worker.thread, NULL, workerFunction, dev);
}

void destroyWorker(worker_thread_t* worker)
{
    workerSetState(worker, STATE_EXIT);
    pthread_join(worker->thread, NULL);

    pthread_cond_destroy(&worker->task_cond);
    pthread_cond_destroy(&worker->idle_cond);
    pthread_mutex_destroy(&worker->mutex);
    worker->request = STATE_NONE;
    worker->state = STATE_NONE;
}

int32_t bl_fp_Init()
{
	BF_LOG("+++");
	/*
    bf_fingerprint_hal_device_t* dev =
        malloc(sizeof(bf_fingerprint_hal_device_t));
    struct bl_fingerprint_data *bl_data;
   
	dev->ree_device = bf_ree_device_new();
	bl_data = bl_fingerprint_data_new(dev->ree_device->devfd);
	init_new_fingerprint_data(bl_data, dev->ree_device->devfd);
	dev->bl_data = bl_data;
	g_bf_hal_fp_dev = dev;
	dev->navState = true;//默认进按键导航模式，如果不是就会进idle模式
	initWorker(dev);
    workerSetState(&dev->worker, STATE_NAVIGATION);
    */
    bf_fingerprint_hal_device_t* dev =
        malloc(sizeof(bf_fingerprint_hal_device_t));
    dev->ree_device = bf_ree_device_new();
	g_bf_hal_fp_dev = dev;
	//imagebuf for debug use
	dev->imagebuf = malloc(DEBUG_IMGBUF_SIZE);

	//tac init
	bf_tac_init(dev);
	//load_config_from file ,debug option,navi func,
    dev->navState = true;
    initWorker(dev);
    pthread_mutex_init(&dev->lock, NULL);
    workerSetState(&dev->worker, STATE_NAVIGATION);
    
	BF_LOG("---");
}
int32_t bl_fp_UnInit()
{
	BF_LOG("+++");

    destroyWorker(&g_bf_hal_fp_dev->worker);
	bf_ree_device_destroy(g_bf_hal_fp_dev->ree_device);
	//destroy_fingerprint_data(g_bf_hal_fp_dev->bl_data);
	bf_tac_uninit();
	BF_LOG("---");
}

int32_t bl_fp_GetRawImage(uint8_t * pBmp, int32_t *params, int32_t *result)
{
	/*
    struct bl_fingerprint_data *bl_data = g_bf_hal_fp_dev->bl_data;
	uint32_t width = 80;
	uint32_t height = 80;
	int value = 0;
    
	width = bl_data->chip_params->width;
	height = bl_data->chip_params->height;
	*/
	uint32_t width = 112;
	uint32_t height = 96;
	memcpy(pBmp, g_bf_hal_fp_dev->imagebuf, height * width);
	//BF_LOG("---");
	return 0;
}
