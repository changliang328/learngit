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
#define NAVI_DETECT_TIME (150)
#define DOUBLE_CLICK_DETECT_TIME (300)
#define NAVI_FP_UP_TIMEOUT	(5*1000) //5S to out capture_navi mode
#define WAIT_FP_UP_TIMEOUT	(1000) 

#define RETRY_COUNT	(3)
#define TAC_TIME_MEASUREMENTS

#define SCREEN_ON	(100)
#define SCREEN_OFF	(101)

#ifndef container_of
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))
#endif

const char *dev_screen_state_name = "/sys/bl_fingerprint_sysfs/screenstate";
const char *dev_workstate_name = "/sys/bl_fingerprint_sysfs/workstate";

struct timeval timer1, timer2;
struct timeval fp_up_timer;
struct timeval naviTimer, navifp_up_timer, double_click_timer;
int time_cost = 0;
int time_wait = 0;
int time_wait_identify_once = 0;

#define USECS_PER_SECOND                1000000
#define USECS_PER_MSEC                  1000

const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_PRESS_FINGER = FINGERPRINT_ACQUIRED_VENDOR_BASE + 1;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_LIFT_FINGER = FINGERPRINT_ACQUIRED_VENDOR_BASE + 2;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = FINGERPRINT_ACQUIRED_VENDOR_BASE + 3;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_DUPLICATE_AREA = FINGERPRINT_ACQUIRED_VENDOR_BASE + 4;


void timer_start(struct timeval* timer)
{
	gettimeofday(timer, NULL);
}


int timer_stop(struct timeval* start)
{
	int time_diff;
	struct timeval stop, delta;

	gettimeofday(&stop, NULL);
	timersub(&stop, start, &delta);
	time_diff = delta.tv_sec * USECS_PER_SECOND + delta.tv_usec;
	return time_diff /  USECS_PER_MSEC;
}

static int workerShouldCancel(worker_thread_t* worker)
{
	pthread_mutex_lock(&worker->mutex);
	int should = (worker->request == STATE_IDLE);
	pthread_mutex_unlock(&worker->mutex);
	return should;
}

static int bf_chip_reinit(bf_fingerprint_hal_device_t* dev)
{
	BF_LOG("++++");
	disable_power_ioctl();
	do_hwreset_ioctl();
	/*
	   set_reset_gpio_low_ioctl();
	   usleep(1000*100);
	   set_reset_gpio_high_ioctl();
	 */
	enable_power_ioctl();
	usleep(1000*100);
	bf_tac_chip_reinit();
	usleep(1000*50);
	clear_pollflag_ioctl();
	BF_LOG("----");
	return 0;
}

static int capture_image(bf_fingerprint_hal_device_t* dev)
{
	bf_capture_data_t capdata;
	int ret = 0;
	int value = 0;
	int retry_count = RETRY_COUNT;

	if(dev->worker.state == STATE_AUTHENTICATE)
	{
		capdata.mode = QUALIFY_MODE_AUTHENTICATE;
	}else
	{
		capdata.mode = QUALIFY_MODE_ENROLL;
	}


	ret = bf_tac_capture_image(&capdata);
	while(retry_count)
	{
		ret = bf_ree_device_wait_irq_paul_timeout(dev->ree_device, FINGERUP_TIME);
		if(ret < 0)
		{
			BF_LOG("ret=%d----",ret);
			if(ret == -EINVAL)
			{
				retry_count--;
				bf_chip_reinit(dev);//abnormal irq come ,may be in ESD,  hwreset
				continue;
			}else if(ret == -EINTR)
			{
				ret = -EINTR;		
			}
			goto out;
		}else{
			break;
		}

	}
	ret = bf_tac_get_intStatus(&value);
	BF_LOG("REGA_INTR_STATUS=%x ", value);
	if(value == 2)
	{
		timer_start(&timer2);
		ret = bf_tac_read_frame((char *)dev->imagebuf);
		time_cost = timer_stop(&timer2);
		LOGD("bf_tac_read_frame took %d ms", time_cost);
		timer_start(&timer2);
		ret = bf_tac_qualify_image(&capdata);
		time_cost = timer_stop(&timer2);
		LOGD("bf_tac_qualify_image took %d ms", time_cost);
		if(capdata.result == 5)
			ret = -EAGAIN;
		else if(capdata.result == BF_LIB_FAIL_LOW_QUALITY)
			ret = BF_LIB_FAIL_LOW_QUALITY;
	}else { 
		ret = -EINVAL;
	}

out:
	return ret;
}

static int capture_image_all(bf_fingerprint_hal_device_t* dev)
{
	bf_capture_data_t capdata;
	int ret = 0;

	if(dev->worker.state == STATE_AUTHENTICATE)
	{
		capdata.mode = QUALIFY_MODE_AUTHENTICATE;
	}else
	{
		capdata.mode = QUALIFY_MODE_ENROLL;
	}

	ret = bf_tac_capture_image_all(&capdata);
	if(BF_LIB_ERROR_SENSOR == ret)
	{
		BF_LOG("BF_LIB_ERROR_SENSOR");
		bf_chip_reinit(dev);
	}

	ret = capdata.result;

out:
	return ret;
}

int32_t do_capture_best_image(bf_fingerprint_hal_device_t* dev)
{
	int32_t status = 0;
	fingerprint_msg_t message = {0, {0}};
	bf_capture_data_t capdata;
	int ret = 0;
	int count = 5;

	if ( NULL == dev )
	{
		LOGE("%s return, reason: dev is NULL", __func__);
		return -EINVAL;
	}

	if(dev->worker.state == STATE_AUTHENTICATE)
	{
		capdata.mode = QUALIFY_MODE_AUTHENTICATE;
	}else
	{
		capdata.mode = QUALIFY_MODE_ENROLL;
	}
	capdata.state = STATE_DO_CAPTURE_START;
	do
	{
		switch(capdata.state)
		{
			case STATE_DO_CAPTURE_START:
				ret = bf_tac_capture_image(&capdata);
				status = capdata.result;
				break;
			case STATE_DO_CAPTURE_GOT_FIRST_FRAME:
			case STATE_DO_CAPTURE_WAIT_FRAMEDONE:
				ret = bf_ree_device_wait_irq_paul_timeout(dev->ree_device, FINGERUP_TIME);
				if(ret < 0)
				{
					BF_LOG("ret=%d----",ret);
					if(ret == -EINVAL)
					{
						bf_chip_reinit(dev);//abnormal irq come ,may be in ESD,  hwreset
						continue;
					}
					goto out;
				}
				bf_tac_capture_image(&capdata);
				status = capdata.result;
				if(count-- <= 0)
				{
					status = -1;
					goto out;
				}
				break;
			case STATE_DO_CAPTURE_GOT_BEST_IMAGE:
				BF_LOG("STATE_DO_CAPTURE_GOT_BEST_IMAGE");
				break;
		}
		if (workerShouldCancel(&dev->worker))
		{
			dev->wait_finger_up = 0;
			LOGD("operation canceled");
			return -EINTR;
		}
		
		if(BF_LIB_ERROR_SENSOR == status)
		{
			BF_LOG("BF_LIB_ERROR_SENSOR");
			bf_chip_reinit(dev);
			continue;
		}
			
		message.type = FINGERPRINT_ACQUIRED;

		if (status == 5)	//image xuEnhance low area
		{
			message.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_PARTIAL;
			dev->device.notify(&message);
			continue;
		}
		else if (status == BF_LIB_FAIL_LOW_QUALITY)
		{
			message.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_INSUFFICIENT;
			dev->device.notify(&message);
			continue;
		}else if (status == BF_LIB_ERROR_SENSOR)
		{
			message.type = FINGERPRINT_ERROR;
			message.data.error = FINGERPRINT_ERROR_HW_UNAVAILABLE;
			dev->device.notify(&message);
			return status;
		}else if(status == BF_LIB_FAIL_INVAILD_TOUCH)
		{
			message.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
			dev->device.notify(&message);
			continue;
		}else
		{
			//continue;
		}
	}
	while (capdata.state != STATE_DO_CAPTURE_GOT_BEST_IMAGE);

out:
	return status;
}


int32_t do_capture_all(bf_fingerprint_hal_device_t* dev)
{
	int32_t status = 0;
	fingerprint_msg_t message = {0, {0}};
	int count = 3;

	if ( NULL == dev )
	{
		LOGE("%s return, reason: dev is NULL", __func__);
		return -EINVAL;
	}
	do
	{
		if(count-- <= 0)
		{
			status = -1;
			break;
		}
		status = capture_image_all(dev);
		//check status if ok
		BF_LOG("status=%d", status);
		message.type = FINGERPRINT_ACQUIRED;

		if (status == 5)	//image xuEnhance low area
		{
			message.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_PARTIAL;
			dev->device.notify(&message);
			continue;
		}
		else if (status == BF_LIB_FAIL_LOW_QUALITY)
		{
			message.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_INSUFFICIENT;
			dev->device.notify(&message);
			continue;
		}else if (status == BF_LIB_ERROR_SENSOR)
		{
			message.type = FINGERPRINT_ERROR;
			message.data.error = FINGERPRINT_ERROR_HW_UNAVAILABLE;
			dev->device.notify(&message);
			return status;
		}else if(status == BF_LIB_FAIL_INVAILD_TOUCH)
		{
			message.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
			dev->device.notify(&message);
			continue;
		}else
		{
			continue;
		}


		if (workerShouldCancel(&dev->worker))
		{
			dev->wait_finger_up = 0;
			LOGD("operation canceled");
			return -EINTR;
		}

	}
	while (status == -EAGAIN);
	BF_LOG("status=%d",status);
	return status;
}

int low_power_deep_sleep(bf_fingerprint_hal_device_t* dev)
{
   return 0;
}

int32_t wait_finger_up(bf_fingerprint_hal_device_t* dev,
		int timeout_ms, int interruptible)
{
	int ret = 0;
	int retry_count = RETRY_COUNT;
	BF_LOG("++++++");
	if(timeout_ms)
		timer_start(&fp_up_timer);
	while(ret != -ETIME)
	{
		clear_pollflag_ioctl();
		bf_tac_fd_mode(FINGER_DETECT_DOWN);
		ret = bf_ree_device_wait_irq_paul_timeout(dev->ree_device, FINGERUP_TIME);
		if(ret < 0)
		{
			BF_LOG("ret=%d----",ret);
			if(ret == -ETIME)
			{
				dev->wait_finger_up = 0;
			}else if(ret == -EINVAL)
			{
				retry_count--;
				bf_chip_reinit(dev);//abnormal irq come ,may be in ESD,  hwreset
				continue;
			}else if(ret == -EINTR)
			{//cancel
				if(interruptible)
				{
					ret = -EINTR;
					break;
				}else{
					//can not cancel ,should clear the cancel pipe,if not can not wait irq any more
					bf_ree_device_clear_cancel(dev->ree_device);
				}

			}
		}

		if(timeout_ms)
		{
			time_cost = timer_stop(&fp_up_timer);
			if(time_cost > timeout_ms)
			{
				ret = -ETIME;
			}
		}
	}
	BF_LOG("ret=%d----",ret);
	return ret;
}

int wait_finger_down(bf_fingerprint_hal_device_t* dev, int mode)
{
	int ret = 0;
	int value = 0;
	int retry_count = RETRY_COUNT;
	BF_LOG("++++");

	while(retry_count)
	{
		clear_pollflag_ioctl();
		bf_tac_fd_mode(FINGER_DETECT_DOWN);
		usleep(5000);
		ret = bf_ree_device_wait_irq_paul(dev->ree_device);
		if(ret < 0)
		{
			BF_LOG("ret=%d----",ret);
			bf_tac_get_intStatus(&value);
			BF_LOG("REGA_INTR_STATUS=%x ", value);
			if(ret == -EINVAL)
			{
				retry_count--;
				bf_chip_reinit(dev);//abnormal irq come ,may be in ESD,  hwreset
				continue;
			}else if(ret == -EINTR)
			{
				return ret;//be canceled
			}

		}else{
			break;
		}
	}
	return ret;
}

#define NAVI_POINT_MAX	(20)
#define NAVI_MOVE_DELTA	(17)
static int bf_determine_navi_dir(navigation_info *data)
{
	int m_dx_max = 0;
	int m_dy_max = 0;
	int m_is_positive_x = 0;
	int m_is_positive_y = 0;

	if(data->deltaX > 0)
	{	
		if(data->deltaX > m_dx_max)
		{
			m_dx_max = data->deltaX;
			m_is_positive_x = 1;
		}
	}
	else
	{
		if(-data->deltaX > m_dx_max)
		{
			m_dx_max = -data->deltaX;
			m_is_positive_x = 0;
		}
	}
	if(data->deltaY > 0)
	{
		if(data->deltaY > m_dy_max)
		{
			m_dy_max = data->deltaY;
			m_is_positive_y = 1;
		}
	}
	else
	{
		if(-data->deltaY > m_dy_max)
		{
			m_dy_max = -data->deltaY;
			m_is_positive_y = 0;
		}
	}
	if((m_dx_max > NAVI_MOVE_DELTA)||(m_dy_max > NAVI_MOVE_DELTA))
	{
		if(m_dx_max > m_dy_max){
			if(m_is_positive_x == 1){
				data->eventValue = EVENT_UP;
			}
			else{
				data->eventValue = EVENT_DOWN;
			}
		}
		else{
			if(m_is_positive_y == 1){
				data->eventValue = EVENT_LEFT;
			}
			else{
				data->eventValue = EVENT_RIGHT;
			}
		}
	}
	return 0;
}
typedef struct bf_navi_point_data
{
	int pointX;
	int pointY;
	int area;
}bf_point_t;
int32_t bf_navi_algo(navigation_info *data)
{
	static bf_point_t points[NAVI_POINT_MAX] = {0};
	int i = 0;
	int timecost = 0;
	int precount = 0;
	int area = data->uAreaCount * 100/ data->height / data->width;

	BF_LOG("X=%d,Y=%d,Area=%d,Mean=%d,deltaX=%d,deltaY=%d",data->uPosX,data->uPosY,area,data->uMeanValue,data->deltaX,data->deltaY);

	switch(data->state)
	{
		case NAVI_STATE_START:
			timecost = timer_stop(&navifp_up_timer);
			BF_LOG("navifp_up_timertimecost=%d",timecost);
			if(timecost > NAVI_FP_UP_TIMEOUT)
			{
				data->state = NAVI_STATE_END;
				break;
			}
			//if(data->uMeanValue < PIXCEL_VALUE_NAVI_PRESS_DOWN)
			if(area > NAVI_FINGER_DOWN_AREA)
			{
				memset(points, 0 , sizeof(bf_point_t) * NAVI_POINT_MAX);
				data->count = 0;
				data->deltaX = 0;
				data->deltaY = 0;
				if(data->eventValue == EVENT_LOST)
				{
					timecost = timer_stop(&double_click_timer);
					if(timecost <= DOUBLE_CLICK_DETECT_TIME)
					{
						//data->eventValue = EVENT_DCLICK;
						BF_LOG("eventValue=%d,Area=%d,Mean=%d,deltaX=%d,deltaY=%d",data->eventValue,area,data->uMeanValue,data->deltaX,data->deltaY);
					}
					BF_LOG("double_click_timer　timecost=%d",timecost);
				}
				data->state = NAVI_STATE_MEASURING;
				points[data->count].pointX = data->uPosX;
				points[data->count].pointY = data->uPosY;
				points[data->count].area = area;
				timer_start(&naviTimer);
			}
			break;
		case NAVI_STATE_MEASURING:
			//if(data->uMeanValue < PIXCEL_VALUE_NAVI_PRESS_DOWN)
			if(area > NAVI_FINGER_DOWN_AREA)
			{
				if((!data->eventValue)||(data->eventValue == EVENT_LOST))
				{
					data->count++;
					data->count %= NAVI_POINT_MAX;
					if(data->count == 0)
					{
						precount = NAVI_POINT_MAX - 1;
					}
					else
					{
						precount = data->count - 1;
					}
					points[data->count].pointX = data->uPosX;
					points[data->count].pointY = data->uPosY;
					points[data->count].area = area;
					data->deltaX += points[data->count].pointX - points[precount].pointX;
					data->deltaY += points[data->count].pointY - points[precount].pointY;

					timecost = timer_stop(&naviTimer);
					if(timecost > NAVI_DETECT_TIME)
					{
						bf_determine_navi_dir(data);
						if((!data->eventValue)||(data->eventValue == EVENT_LOST))
						{
							if(timecost > DOUBLE_CLICK_DETECT_TIME)
							{
								//data->eventValue = EVENT_HOLD;
								data->eventValue = EVENT_CLICK;
							}
						}
					}	

					BF_LOG("eventValue=%d,Area=%d,Mean=%d,deltaX=%d,deltaY=%d",data->eventValue,area,data->uMeanValue,data->deltaX,data->deltaY);
				}
			}else
			{
				if((!data->eventValue)||(data->eventValue == EVENT_LOST))
				{
					timecost = timer_stop(&naviTimer);
					BF_LOG("naviTimer  timecost=%d",timecost);
					if(timecost > FINGERUP_TIME)
					{
						bf_determine_navi_dir(data);
						if((!data->eventValue)||(data->eventValue == EVENT_LOST)){
							data->eventValue = EVENT_CLICK;
						}
					}else
					{
						data->eventValue = EVENT_CLICK;
					}	

					if((data->eventValue)&&(data->eventValue !=  EVENT_LOST) )
					{
						if(data->lastevent  !=  data->eventValue)
						{
							data->lastevent=data->eventValue;
							bf_key_event_ioctl(data);
						}
					}
				}
				BF_LOG("data->count COUNT=%d",data->count);
				for(i = 0; i < data->count; i++)
				{
					BF_LOG("eventValue[%d]X=%d,Y=%d,area=%d",i,points[i].pointX,points[i].pointY,points[i].area);
				}

				data->eventValue = EVENT_LOST;
				data->state = NAVI_STATE_START;
				timer_start(&navifp_up_timer);
				memcpy(&double_click_timer,&naviTimer,sizeof(struct timeval));
			}
			break;
		case NAVI_STATE_END:
			break;
	}
	return data->state;
}

int32_t do_navigation(bf_fingerprint_hal_device_t* dev, int request)
{
	int ret = 0;
	int value = 0;
	int retry_count = 50;//RETRY_COUNT;

	int timecost = 0;
	navigation_info navidata = {0};

	navidata.state = NAVI_STATE_START;
	clear_pollflag_ioctl();
	ret = wait_finger_down(dev, 0);
	if(ret < 0)
	{
		if(ret == -EINVAL)
		{
			//abnormal irq come ,may be in ESD,  hwreset
		}else if(ret == -EINTR)
		{
			//be canceled
		}
		goto out;
	} 
	navidata.lastevent = 0;
	navidata.eventValue = 0;
	timer_start(&navifp_up_timer);
	while((navidata.state != NAVI_STATE_END) && (retry_count))
	{
		ret = bf_tac_get_navigation_event(&navidata);
		if(navidata.result == 0)
		{
			bf_navi_algo(&navidata);
			if((navidata.eventValue)&&(navidata.lastevent != navidata.eventValue))
			{
				navidata.lastevent = navidata.eventValue;
				bf_key_event_ioctl(&navidata);
			}

		}else
		{
			retry_count--;
			bf_chip_reinit(dev);//abnormal irq come ,may be in ESD,  hwreset
			continue;
		}

		if(navidata.state == NAVI_STATE_END)
			break;

		if (workerShouldCancel(&dev->worker))
		{
			BF_LOG("[bf_navi] cancel exit ");
			ret = -EINTR;
			break;
		}

		ret = bf_ree_device_wait_irq_paul_timeout(dev->ree_device, FINGERUP_TIME);
		if(ret < 0)
		{
			BF_LOG("ret=%d----",ret);
			if(ret == -EINVAL)
			{
				retry_count--;
				bf_chip_reinit(dev);//abnormal irq come ,may be in ESD,  hwreset
				continue;
			}//else -ETIME -EINTR
			goto out;
		}
	}
	BF_LOG("data->eventValue = %d",navidata.eventValue);
out:
	return ret;
}

void do_identify_Interrupt(bf_fingerprint_hal_device_t* dev)
{
	int ret = 0;
	bf_identify_data_t identify_data = {0};
	fingerprint_msg_t acqu_msg_keep_awake = {0};
	fingerprint_msg_t msg = {0};
	identify_data.index = -1;
	
	ret = wait_finger_up(dev, 0, 1);
	if((ret < 0) && (ret != -ETIME))
	{
		goto out;//cancel
	}
	
	for(;;)
	{
		ret = wait_finger_down(dev, 0);
		if(ret < 0)
		{
			if(ret == -EINVAL)
			{
				//abnormal irq come ,may be in ESD,  hwreset
				continue;
			}else if(ret == -EINTR)
			{
				//be canceled
			}
			goto out;
		} 

		acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
		acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
		dev->device.notify(&acqu_msg_keep_awake);
#ifdef TAC_TIME_MEASUREMENTS
		timer_start(&timer1);
#endif
		ret = do_capture_best_image(dev);
		
		if(0 == ret)
		{
			ret = bf_tac_identify(&identify_data);
		}
		else
		{
			BF_LOG("ret=%d----",ret);
			if(ret == -EINTR)
				goto out;
			else
				continue;//else -ETIME -EINVAL
		}
#ifdef TAC_TIME_MEASUREMENTS
		time_cost = timer_stop(&timer1);
		LOGD("do_identify_Interrupt took %d ms,result=%d,index=%d", time_cost,identify_data.result,identify_data.index);
#endif
		msg.type = FINGERPRINT_AUTHENTICATED;
		if(identify_data.index >= 0)
		{
			msg.data.authenticated.finger.gid = dev->current_gid;
			msg.data.authenticated.finger.fid = identify_data.matchID;
			//necessary
			msg.data.authenticated.hat.version = HW_AUTH_TOKEN_VERSION;
			msg.data.authenticated.hat.challenge = dev->operation_id;
			msg.data.authenticated.hat.user_id= dev->hat.user_id;  
			msg.data.authenticated.hat.authenticator_id = dev->authenticator_id;
			msg.data.authenticated.hat.authenticator_type = htobe32(HW_AUTH_FINGERPRINT);
			BF_LOG("hat.authenticator_id =%llu",msg.data.authenticated.hat.authenticator_id);

			//get token tac function
			bf_tac_get_auth_token(&(msg.data.authenticated.hat));
#if 0
			BF_LOG("gid =%d", dev->current_gid);
			BF_LOG("hat.version =%u",msg.data.authenticated.hat.version);
			BF_LOG("hat.user_id =%llu",msg.data.authenticated.hat.user_id);
			BF_LOG("hat.authenticator_id =%llu",msg.data.authenticated.hat.authenticator_id);
			BF_LOG("hat.authenticator_type = %u",msg.data.authenticated.hat.authenticator_type);
			BF_LOG("hat.challenge =%llu ",msg.data.authenticated.hat.challenge);
			BF_LOG("hat.timestamp =%llu   sizeof_token=%d\n",msg.data.authenticated.hat.timestamp,sizeof(msg.data.authenticated.hat));
#endif

			dev->device.notify(&msg);
			bf_tac_update_template_indic(identify_data.indic);
			ret = wait_finger_up(dev, 0, 1);
			if((ret < 0) && (ret != -ETIME))
			{
				goto out;//cancel
			}
			break;
		}
		else
		{
			msg.data.authenticated.finger.gid = dev->current_gid;
			msg.data.authenticated.finger.fid = 0;
			dev->device.notify(&msg);
		}
		ret = wait_finger_up(dev, 0, 1);
		if((ret < 0) && (ret != -ETIME))
		{
			goto out;//cancel
		}
	}
out:
	return ;
}

void do_identify_AllInOne(bf_fingerprint_hal_device_t* dev)
{
	int ret = 0;
	bf_identify_data_t identify_data = {0};
	fingerprint_msg_t acqu_msg_keep_awake = {0};
	fingerprint_msg_t msg = {0};
	identify_data.index = -1;
	
	ret = wait_finger_up(dev, 0, 1);
	if((ret < 0) && (ret != -ETIME))
	{
		goto out;//cancel
	}
	
	for(;;)
	{
		ret = wait_finger_down(dev, 0);
		if(ret < 0)
		{
			if(ret == -EINVAL)
			{
				//abnormal irq come ,may be in ESD,  hwreset
				continue;
			}else if(ret == -EINTR)
			{
				//be canceled
			}
			goto out;
		} 

		acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
		acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
		dev->device.notify(&acqu_msg_keep_awake);
#ifdef TAC_TIME_MEASUREMENTS
		timer_start(&timer1);
#endif
		ret = bf_tac_identify_all(&identify_data);
#ifdef TAC_TIME_MEASUREMENTS
		time_cost = timer_stop(&timer1);
		LOGD("do_identify_AllInOne took %d ms,result=%d,index=%d", time_cost,identify_data.result,identify_data.index);
#endif
		msg.type = FINGERPRINT_AUTHENTICATED;
		if((identify_data.result == 0) && (identify_data.index >= 0))
		{
			msg.data.authenticated.finger.gid = dev->current_gid;
			msg.data.authenticated.finger.fid = identify_data.matchID;
			//necessary
			msg.data.authenticated.hat.version = HW_AUTH_TOKEN_VERSION;
			msg.data.authenticated.hat.challenge = dev->operation_id;
			msg.data.authenticated.hat.user_id= dev->hat.user_id;  
			msg.data.authenticated.hat.authenticator_id = dev->authenticator_id;
			msg.data.authenticated.hat.authenticator_type = htobe32(HW_AUTH_FINGERPRINT);

			//get token tac function
			bf_tac_get_auth_token(&(msg.data.authenticated.hat));
#if 0
			BF_LOG("gid =%d", dev->current_gid);
			BF_LOG("hat.version =%u",msg.data.authenticated.hat.version);
			BF_LOG("hat.user_id =%llu",msg.data.authenticated.hat.user_id);
			BF_LOG("hat.authenticator_id =%llu",msg.data.authenticated.hat.authenticator_id);
			BF_LOG("hat.authenticator_type = %u",msg.data.authenticated.hat.authenticator_type);
			BF_LOG("hat.challenge =%llu ",msg.data.authenticated.hat.challenge);
			BF_LOG("hat.timestamp =%llu   sizeof_token=%d\n",msg.data.authenticated.hat.timestamp,sizeof(msg.data.authenticated.hat));
#endif

			dev->device.notify(&msg);
			bf_tac_update_template_indic(identify_data.indic);
			ret = wait_finger_up(dev, 0, 1);
			if((ret < 0) && (ret != -ETIME))
			{
				goto out;//cancel
			}
			break;
		}else if(identify_data.result == BF_LIB_FAIL_LOW_QUALITY)
		{
			acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
			acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_INSUFFICIENT;
			dev->device.notify(&acqu_msg_keep_awake);
			continue;
		}else if(identify_data.result == BF_LIB_FAIL_INVAILD_TOUCH)
		{
			acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
			acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
			dev->device.notify(&acqu_msg_keep_awake);
			continue;
		}else if(identify_data.result == 5)
		{
			acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
			acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_PARTIAL;
			dev->device.notify(&acqu_msg_keep_awake);
			continue;			
		}else if(BF_LIB_ERROR_SENSOR == identify_data.result)
		{
			BF_LOG("BF_LIB_ERROR_SENSOR");
			bf_chip_reinit(dev);
		}
		else
		{
			msg.data.authenticated.finger.gid = dev->current_gid;
			msg.data.authenticated.finger.fid = 0;
			dev->device.notify(&msg);
		}
		ret = wait_finger_up(dev, 0, 1);
		if((ret < 0) && (ret != -ETIME))
		{
			goto out;//cancel
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
			//notify_enroll_cancel(dev);
			break;
		case CANCEL_AUTHENTICATE:
			//notify_authenticate_cancel(dev);
			break;
		default:
			break;
	}
}

void do_enroll_interrupt(bf_fingerprint_hal_device_t* dev)
{
	int ret = 0;
	int value = 0;
	int lastenroll = 0;
	bf_enroll_data_t enrolldata = {0};
	bf_identify_data_t identify_data = {0};

	fingerprint_msg_t acqu_msg_keep_awake = {0};
	fingerprint_msg_t msg = {0};

	u32 fid = 0;
	int status = 0;
	uint32_t indices_count = 0;
	uint32_t indices[BF_MAX_FINGER];
	enrolldata.required_samples = dev->required_samples;
	status = bf_tac_get_indices(dev->tac_handle, indices, &indices_count);
	bf_tac_new_fid(&fid);

	for(;;)
	{
		identify_data.index = -1;
		ret = wait_finger_down(dev, 0);
		if(ret < 0)
		{
			if(ret == -EINVAL)
			{
				continue;
				//abnormal irq come ,may be in ESD,  hwreset
			}else if(ret == -EINTR)
			{
				//be canceled
			}
			goto out;
		} 
		acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
		acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
		dev->device.notify(&acqu_msg_keep_awake);
		//check status if ok
#ifdef TAC_TIME_MEASUREMENTS
		timer_start(&timer1);
#endif
		ret = do_capture_best_image(dev);
#ifdef TAC_TIME_MEASUREMENTS
		time_cost = timer_stop(&timer1);
		LOGD("do_enroll capture took %d ms", time_cost);
#endif
		if(ret == 0)
		{
			if(indices_count > 0)
			{
				ret = bf_tac_identify(&identify_data);
				BF_LOG("ret=%d index=%d",ret, identify_data.index);
				if(identify_data.index != -1)
				{//have enrolled this finger
					acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
					acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_DUPLICATE_FINGER;
					dev->device.notify(&acqu_msg_keep_awake);
					wait_finger_up(dev, 0, 1);
					continue;
				}

#ifdef TAC_TIME_MEASUREMENTS
				time_cost = timer_stop(&timer1);
				LOGD("do_enroll identify took %d ms", time_cost);
#endif
			}

			//enroll one frame
			if((ret == 0) && (identify_data.index == -1))
			{
				ret = bf_tac_enroll(&enrolldata);
#ifdef TAC_TIME_MEASUREMENTS
				time_cost = timer_stop(&timer1);
				LOGD("do_enroll enroll took %d ms", time_cost);
#endif
				if(lastenroll != enrolldata.progress)
				{
					msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
					msg.data.enroll.finger.fid = fid;
					msg.data.enroll.finger.gid = dev->current_gid;
					msg.data.enroll.samples_remaining = enrolldata.required_samples - enrolldata.progress;

					lastenroll = enrolldata.progress;
					dev->device.notify(&msg);
					if(msg.data.enroll.samples_remaining == 0)
					{	
						wait_finger_up(dev, 0, 1);
						goto out;
					}

				}else if(lastenroll == enrolldata.progress)	//duplicat area
				{
					acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
					acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_DUPLICATE_AREA;
					dev->device.notify(&acqu_msg_keep_awake);
					continue;
				}
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

void do_enroll_all(bf_fingerprint_hal_device_t* dev)
{
	int ret = 0;
	int value = 0;
	int lastenroll = 0;
	bf_enroll_data_t enrolldata = {0};
	bf_identify_data_t identify_data = {0};

	fingerprint_msg_t acqu_msg_keep_awake = {0};
	fingerprint_msg_t msg = {0};

	u32 fid = 0;
	int status = 0;
	uint32_t indices_count = 0;
	uint32_t indices[BF_MAX_FINGER];
	enrolldata.required_samples = dev->required_samples;
	status = bf_tac_get_indices(dev->tac_handle, indices, &indices_count);
	bf_tac_new_fid(&fid);

	for(;;)
	{
		identify_data.index = -1;
		ret = wait_finger_down(dev, 0);
		if(ret < 0)
		{
			if(ret == -EINVAL)
			{
				//abnormal irq come ,may be in ESD,  hwreset
			}else if(ret == -EINTR)
			{
				//be canceled
			}
			goto out;
		} 
		acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
		acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
		dev->device.notify(&acqu_msg_keep_awake);
		//check status if ok
#ifdef TAC_TIME_MEASUREMENTS
		timer_start(&timer1);
#endif
		ret = do_capture_all(dev);
#ifdef TAC_TIME_MEASUREMENTS
		time_cost = timer_stop(&timer1);
		LOGD("do_enroll capture took %d ms", time_cost);
#endif
		if(ret == 0)
		{
			if(indices_count > 0)
			{
				ret = bf_tac_identify(&identify_data);
				BF_LOG("ret=%d index=%d",ret, identify_data.index);
				if(identify_data.index != -1)
				{//have enrolled this finger
					acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
					acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_DUPLICATE_FINGER;
					dev->device.notify(&acqu_msg_keep_awake);
					wait_finger_up(dev, 0, 1);
					continue;
				}

#ifdef TAC_TIME_MEASUREMENTS
				time_cost = timer_stop(&timer1);
				LOGD("do_enroll identify took %d ms", time_cost);
#endif
			}

			//enroll one frame
			if((ret == 0) && (identify_data.index == -1))
			{
				ret = bf_tac_enroll(&enrolldata);
#ifdef TAC_TIME_MEASUREMENTS
				time_cost = timer_stop(&timer1);
				LOGD("do_enroll enroll took %d ms", time_cost);
#endif
				if(lastenroll != enrolldata.progress)
				{
					msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
					msg.data.enroll.finger.fid = fid;
					msg.data.enroll.finger.gid = dev->current_gid;
					msg.data.enroll.samples_remaining = enrolldata.required_samples - enrolldata.progress;

					lastenroll = enrolldata.progress;
					dev->device.notify(&msg);
					if(msg.data.enroll.samples_remaining == 0)
					{	
						wait_finger_up(dev, 0, 1);
						goto out;
					}

				}else if(lastenroll == enrolldata.progress)	//duplicat area
				{
					acqu_msg_keep_awake.type = FINGERPRINT_ACQUIRED;
					acqu_msg_keep_awake.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_DUPLICATE_AREA;
					dev->device.notify(&acqu_msg_keep_awake);
					continue;
				}
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
{
	int result = 0;
	int status = 0;
	uint32_t value = 0;
	mmi_info *pMmiInfo = &dev->tMmiInfo;
	mmi_test_params_t *mmi_data = pMmiInfo->mmi_data;
	uint32_t *pudacp;
	int isExit = 0;
	
	while(1)
	{
		pthread_mutex_lock(&pMmiInfo->mmi_mutex);
		if(dev->worker.request == STATE_IDLE)
		{
			isExit = 1;

		}else 
		{
			if(1 == pMmiInfo->isTesting)
			{
				pthread_cond_wait(&pMmiInfo->mmi_taskcond, &pMmiInfo->mmi_mutex);
			}
			else
				pMmiInfo->isTesting = 1;
		}
		
		if(isExit)
		{
			BF_LOG("operation canceled");
			pthread_cond_signal(&pMmiInfo->mmi_idlecond);
			pthread_mutex_unlock(&pMmiInfo->mmi_mutex);
			pMmiInfo->testResult = result = -EINTR;
			break;
		}
		
		switch (pMmiInfo->testType)
		{
			case MMI_TYPE_AUTO_TEST:
		        BF_LOG("MMI_AUTO_TEST");
		        disable_power_ioctl();
				usleep(1000*100);
		        enable_power_ioctl();
				//1.reset value should low
				set_reset_gpio_low_ioctl();
				value = get_reset_gpio_value_ioctl();
				BF_LOG("MMI_AUTO_TEST reset low test reset_gpio_value=%d",value);
		        if (0 == value)
		        { result = BF_MMI_TEST_OK; }
		        else
		        { 
		        	result = BF_MMI_TEST_FAILED;
		            break;
		        }
				usleep(1000*100);
				//2.reset value should high
				set_reset_gpio_high_ioctl();
				value = get_reset_gpio_value_ioctl();
				BF_LOG("MMI_AUTO_TEST reset high test reset_gpio_value=%d",value);
		        if(1 == value)
		        { 	result = BF_MMI_TEST_OK; }
		        else
		        { 
		        	result = BF_MMI_TEST_FAILED;
		            break;
		        }
				//3.irq value
				value = get_irq_gpio_value_ioctl();
				BF_LOG("MMI_AUTO_TEST irq high test irq_gpio_value=%d",value);
		        if(1 == value)
		        { 	result = BF_MMI_TEST_OK; }
		        else
		        { 
		        	result = BF_MMI_TEST_FAILED;
		            break;
		        }
				
				usleep(1000*100);
				result = bf_tac_chip_reinit();
		        BF_LOG("MMI_AUTO_TEST bf_tac_chip_reinit result=%d", result);
				usleep(1000*50);
				clear_pollflag_ioctl();
				
				//4.idle mode ,int gpio should low
				pMmiInfo->mmi_data->action = BF_MMI_ACT_SET_IDLE_MODE;
		        bf_tac_do_mmi_test(pMmiInfo->mmi_data);
		        value = get_irq_gpio_value_ioctl();
				BF_LOG("MMI_AUTO_TEST irq low test irq_gpio_value=%d",value);
		        if(0 == value)
		        { 	result = BF_MMI_TEST_OK; }
		        else
		        {
		            result = BF_MMI_TEST_FAILED;
		            break;
		        }
		        
				//5.capture mode
				pMmiInfo->mmi_data->action = BF_MMI_ACT_CAPTURE_WITH_DACP;
		        bf_tac_do_mmi_test(pMmiInfo->mmi_data);
				status = bf_ree_device_wait_irq_paul_timeout(dev->ree_device, FINGERUP_TIME);
		        if(0 == status)
		        { 	result = BF_MMI_TEST_OK; }
		        else
		        {
		            result = BF_MMI_TEST_FAILED;
		            break;
		        }
				
				//6.get int status
		       	bf_tac_get_intStatus(&value);
		        BF_LOG("MMI_AUTO_TEST bf_tac_get_intStatus status=%d", value);
		        if(BF_INTSTATE_FRAME_DONE == value)
		        { result = BF_MMI_TEST_OK; }
		        else
		        { 
		        	result = BF_MMI_TEST_FAILED;
		            break;
		        }
		        
				break;

		    case MMI_TYPE_INTERRUPT_TEST:
		        status = wait_finger_down(dev, 0);

		        if (status == 0)
		        { result = BF_MMI_TEST_OK; }
		        else if (status == -EINTR)
		        {
		            result = BF_MMI_CANCEL;
		            break;
		        }
		        else
		        { result = BF_MMI_TEST_FAILED; }

		        break;

		    case MMI_TYPE_GET_RAWIMAGE_WITHOUT_INT:
		        //status = wait_finger_down(dev, 0);
				pMmiInfo->mmi_data->action = BF_MMI_ACT_CAPTURE_WITH_DACP;
				pudacp = (uint32_t *)&mmi_data->params[0];
				mmi_data->length = dev->width*dev->height;
				if(pMmiInfo->testType == get_workstate_cmd(dev->workstate))
					*pudacp = get_workstate_arg1(dev->workstate);
				BF_LOG("MMI_GET_RAWIMAGE_WITHOUT_INT *pudacp=%d,mmi_data->params[0]=%d",*pudacp ,mmi_data->params[0]);
		        bf_tac_do_mmi_test(pMmiInfo->mmi_data);
		        if (status == -EINTR)
		        {
		            result = BF_MMI_CANCEL;
		            break;
		        }
				status = bf_ree_device_wait_irq_paul_timeout(dev->ree_device, FINGERUP_TIME);
		        if (status == -EINTR)
		        {
		            result = BF_MMI_CANCEL;
		            break;
		        }
				pMmiInfo->mmi_data->action = BF_MMI_ACT_GET_RAW_IMAGE;
		        bf_tac_do_mmi_test(pMmiInfo->mmi_data);
		        if (status != 0)
		        {
		            result = BF_MMI_TEST_FAILED;
		            break;
		        }
		        result = BF_MMI_TEST_OK; 
		        write_image_to_driver(mmi_data->params, mmi_data->length);
		        break;

		    case MMI_TYPE_GET_RAWIMAGE_WITH_INT:
		        status = wait_finger_down(dev, 0);
		        pudacp = mmi_data->params;
				if(pMmiInfo->testType == get_workstate_cmd(dev->workstate))
					*pudacp = get_workstate_arg1(dev->workstate);
				mmi_data->length = dev->width*dev->height;
		        BF_LOG("MMI_GET_RAWIMAGE_WITH_INT　status=%d,*pudacp=%d",status,*pudacp);
				pMmiInfo->mmi_data->action = BF_MMI_ACT_CAPTURE_WITH_DACP;
		        bf_tac_do_mmi_test(pMmiInfo->mmi_data);
		        if (status == -EINTR)
		        {
		            result = BF_MMI_CANCEL;
		            break;
		        }
				status = bf_ree_device_wait_irq_paul_timeout(dev->ree_device, FINGERUP_TIME);
		        if (status == -EINTR)
		        {
		            result = BF_MMI_CANCEL;
		            break;
		        }
				pMmiInfo->mmi_data->action = BF_MMI_ACT_GET_RAW_IMAGE;
		        bf_tac_do_mmi_test(pMmiInfo->mmi_data);
		        if (status != 0)
		        {
		            result = BF_MMI_TEST_FAILED;
		            break;
		        }
		        result = BF_MMI_TEST_OK; 
		        write_image_to_driver(mmi_data->params, mmi_data->length);
		        break;
		        
		    case MMI_TYPE_AUTO_GET_BEST_IMAGE:
		        BF_LOG("MMI_TYPE_AUTO_GET_BEST_IMAGE");
		        status = wait_finger_down(dev, 0);
				mmi_data->length = dev->width*dev->height;
		        if (status == 0)
		        { result = BF_MMI_TEST_OK; }
		        else if (status == -EINTR)
		        {
		            result = BF_MMI_CANCEL;
		            break;
		        }
		        else
		        { 
		        	result = BF_MMI_TEST_FAILED;
		            break;
		        }
		        status = do_capture_best_image(dev);
		        if (status == 0)
		        { result = BF_MMI_TEST_OK; }
		        else if (status == -EINTR)
		        {
		            result = BF_MMI_CANCEL;
		            break;
		        }
		        else
		        { 
		        	result = BF_MMI_TEST_FAILED;
		        	break;
		        }
				pMmiInfo->mmi_data->action = BF_MMI_ACT_GET_RAW_IMAGE;
		        bf_tac_do_mmi_test(pMmiInfo->mmi_data);
		        if (status != 0)
		        {
		            result = BF_MMI_TEST_FAILED;
		            break;
		        }
		        result = BF_MMI_TEST_OK; 
		        write_image_to_driver(mmi_data->params, mmi_data->length);
		        break;
			case MMI_TYPE_GET_HEIGHT_WIDTH:
				value = dev->workstate;
				value = set_workstate_arg1(value, dev->width);
				value = set_workstate_arg2(value, dev->height);
				dev->workstate = value;
				result = BF_MMI_TEST_OK;
				break;
		   case MMI_TYPE_SNR_WHITE_IMAGE_TEST:
		        status = wait_finger_up(dev, 0, 1);

		        if (status == 0 || status == -EINTR)
		        { result = BF_MMI_TESTING; }

		        else
		        { result =  (uint32_t)BF_MMI_TEST_FAILED; }

		        LOGI("MMI_SNR_WHITE_IMAGE_TEST end\n");
		        break;

		    default:
		        LOGE("invaild mmi_test command \n");
		        break;
		}
		dev->workstate = set_workstate_result(dev->workstate, result);
		set_dev_mmi_workstate(dev->workstate);
		BF_LOG("dev->workstate=0x%x",dev->workstate);
		
		pMmiInfo->testResult = result;
		pthread_cond_signal(&pMmiInfo->mmi_idlecond);
		pthread_mutex_unlock(&pMmiInfo->mmi_mutex);
    }
	pMmiInfo->isTesting = 0;
	return result;
}

int bf_start_mmi_test(bf_fingerprint_hal_device_t *dev)
{
	mmi_info *pMmiInfo = &dev->tMmiInfo;
	pthread_mutex_lock(&pMmiInfo->mmi_mutex);

	if(dev->worker.state != STATE_MMITEST)
	{
		pthread_mutex_lock(&dev->lock);
		workerSetState(&dev->worker, STATE_MMITEST);
		pthread_mutex_unlock(&dev->lock);
    }
	if(1 == pMmiInfo->isTesting)
	{
		pthread_cond_signal(&pMmiInfo->mmi_taskcond);
	}
	pthread_cond_wait(&pMmiInfo->mmi_idlecond, &pMmiInfo->mmi_mutex); 
	pthread_mutex_unlock(&pMmiInfo->mmi_mutex);
}

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
worker_state_t laststate;
int workerSetState(worker_thread_t* worker, worker_state_t state)
{
	bf_fingerprint_hal_device_t* hal_dev = NULL;

	/* Get the hal device handle from worker pointer */
	hal_dev = container_of(worker, bf_fingerprint_hal_device_t,
			worker);
	if(STATE_EXIT == state)
	{
		hal_dev->workstate = STATE_EXIT;
	}
	BF_LOG("+++%d",state);
	pthread_mutex_lock(&worker->mutex);
	/*
	   if(hal_dev->wait_finger_up == 1)
	   {
	   laststate = state;
	   BF_LOG("laststate=%d",state);
	   pthread_mutex_unlock(&worker->mutex);
	   return 0;
	   }
	 */
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

		//cancle the wait_irq
		bf_ree_device_set_cancel(hal_dev->ree_device);
		if(1 == hal_dev->tMmiInfo.isTesting)
		{
			pthread_cond_signal(&hal_dev->tMmiInfo.mmi_taskcond);
		}
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
#define READ_BUF_SIZE	(16)
void* getScreenStateWorkerFunction(void* data)
{
	bf_fingerprint_hal_device_t* dev = (bf_fingerprint_hal_device_t*) data;
	mmi_info *pMmiInfo = &dev->tMmiInfo;
	LOGE("enter getScreenStateWorkerFunction \n");  
	struct pollfd pollfds[2]={0};
	int timeout = -1;  
	char read_buf[READ_BUF_SIZE];
	int status=-1;
	int err=-1;
	int screenfd=-1;      
	int workfd=-1;         
	while(1)
	{
		dev->debug += 1;
		pollfds[0].revents =  0;                  //发生的事件
		pollfds[0].events =   POLLERR | POLLPRI;        //关心，感兴趣的事件
		pollfds[1].revents =  0;               
		pollfds[1].events =   POLLERR | POLLPRI;
		memset(read_buf,0,READ_BUF_SIZE);		
		//open sys file
		screenfd = open(dev_screen_state_name, O_RDONLY|O_NONBLOCK); 
		pollfds[0].fd = screenfd;     
		if(screenfd<0)
		{
			LOGE("open /sys/bl_fingerprint_sysfs/screenstate   fail:  please check permission or node exist ? \r\n");  
			return (void*)(-1);
		}
		
		workfd = open(dev_workstate_name, O_RDONLY); 
		pollfds[1].fd = workfd;     
		if(workfd<0)
		{
			LOGE("open /sys/bl_fingerprint_sysfs/workstate   fail:  please check permission or node exist ? \r\n");  
			return (void*)(-1);
		}
		
		//read sys file
		err=read(screenfd, read_buf, READ_BUF_SIZE);
		if(err < 0){
			LOGE("read  screenfd error");  
		} 

		if(strcmp(read_buf,"100")==0)  
		{
			LOGE("read_ret=%d     screen on  \r\n",err);
			dev->screenstate = SCREEN_ON;
		}
		else if(strcmp(read_buf,"101")==0)	
		{
			LOGE("read_ret=%d     screen off  \r\n",err);
			dev->screenstate = SCREEN_OFF;
		}
		
		memset(read_buf,0,READ_BUF_SIZE);		
		err=read(workfd, read_buf, READ_BUF_SIZE);
		if(err < 0){
			LOGE("read  screenfd error");  
		} 
		
		dev->workstate = strtoul(read_buf, NULL, 16);
		BF_LOG("read_buf=%s ,workstate = %x",read_buf, dev->workstate);
		//poll to wait sysfs_notify from driver
		status = poll(pollfds, 2,timeout);   

		BF_LOG("pollfds[0].revents=0x%x",pollfds[0].revents); 
		BF_LOG("pollfds[1].revents=0x%x",pollfds[1].revents); 
		close(screenfd);
		close(workfd);
		if (status == -1)
        {
            status = -errno;
            LOGE("bf poll irq error");
            goto out;
        }else if (status == 0)
        {
        	status = -ETIME;
            LOGE("bf poll irq timeout");
            goto out;
        }else if (pollfds[0].revents != 0)
        {
        
        }else if (pollfds[1].revents != 0)
        {
        	BF_LOG("MMI_TEST+++");
        	workfd = open(dev_workstate_name, O_RDONLY);
			memset(read_buf,0,READ_BUF_SIZE);		
			err=read(workfd, read_buf, READ_BUF_SIZE);
			if(err < 0){
				LOGE("read  screenfd error");  
			} 
		
			dev->workstate = strtoul(read_buf, NULL, 16);
			BF_LOG("read_buf=%s ,workstate = %x",read_buf, dev->workstate);
			close(workfd);

			pMmiInfo->testType = MMI_TYPE_AUTO_TEST;
			mmi_test_params_t *pmmi_params = (mmi_test_params_t *)dev->imagebuf;
			pMmiInfo->mmi_data = pmmi_params;
			pmmi_params->action = BF_MMI_ACT_MAX;
			pmmi_params->cmd = BF_CMD_DO_MMI_TEST;

        	if(get_workstate_cmd(dev->workstate) != 0)
        	{
				pMmiInfo->testType = get_workstate_cmd(dev->workstate);
				bf_start_mmi_test(dev);
			}else
			{
				BF_LOG("read_buf=%s ,workstate = %d",read_buf, dev->workstate);
			}

        	BF_LOG("MMI_TEST+++result=%d, dev->workstate=%x",pMmiInfo->testResult,dev->workstate);
        }
        
        if(dev->workstate == STATE_EXIT) 
        	goto out;
	}
out:
	BF_LOG("exit workstate = %x", dev->workstate);
	dev->workstate = 0;
	return status;
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
		LOGE("debug = %d",dev->debug);
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

				//do_enroll_all(dev);
				do_enroll_interrupt(dev);
				break;

			case STATE_AUTHENTICATE:
				LOGD("%s STATE_AUTHENTICATE\n", __func__);

				//do_identify(dev);
				//do_identify_AllInOne(dev);
				do_identify_Interrupt(dev);
				break;

			case STATE_NONE:
				break;

			case STATE_NAVIGATION:
				LOGD("%s STATE_NAVIGATION\n", __func__);
				ret = do_navigation(dev, STATE_NAVIGATION);
				//ret = do_navigation_all(dev, STATE_NAVIGATION);
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

int32_t mmi_info_init(bf_fingerprint_hal_device_t* dev)
{
	pthread_mutex_init(&dev->tMmiInfo.mmi_mutex, NULL);
	pthread_cond_init(&dev->tMmiInfo.mmi_taskcond, NULL);
	pthread_cond_init(&dev->tMmiInfo.mmi_idlecond, NULL);
	dev->tMmiInfo.testType = MMI_TEST_NONE;
	dev->tMmiInfo.testResult = 0;
	dev->tMmiInfo.isTesting = 0;
	return 0;
}

int32_t initGetScreenStateWorker(bf_fingerprint_hal_device_t* dev)
{
	return pthread_create(&dev->worker.getScreenStateThread, NULL, getScreenStateWorkerFunction, dev);
}

void destroyWorker(worker_thread_t* worker)
{
	workerSetState(worker, STATE_EXIT);
	pthread_join(worker->thread, NULL);
	//pthread_join(worker->getScreenStateThread, NULL);
	pthread_cond_destroy(&worker->task_cond);
	pthread_cond_destroy(&worker->idle_cond);
	pthread_mutex_destroy(&worker->mutex);
	worker->request = STATE_NONE;
	worker->state = STATE_NONE;
}

int bf_get_finger_data(bf_ca_app_data_t * fpdata)
{
	return bf_tac_get_finger_data(fpdata);
}

