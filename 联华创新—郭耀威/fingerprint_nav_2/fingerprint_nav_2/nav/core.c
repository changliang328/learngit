/******************** (C) COPYRIGHT 2017 BTL ********************************
* File Name          : core.c
* Author               : guoyaowei
* Version              : 1.0
* Date                  : 2017.2.21
*******************************************************************************/

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include <linux/input.h>  
#include <linux/uinput.h>

#include <cutils/properties.h>

#include "log.h"
#include "nav.h"

#define TAG 	"nav_core"

fingerprint_param_nav_t  gFingerprintParamNav = {0};
fingerprint_var_nav_t    gFingerprintVarNav = {0};

fingerprint_param_nav_t  gFingerprintParamNavCtl = {0};
fingerprint_var_nav_t    gFingerprintVarNavCtl = {0};
fingerprint_param_nav_t  gFingerprintParamNavReport = {0};
fingerprint_var_nav_t    gFingerprintVarNavReport = {0};
char inNavReporting = 0;
uint8_t image[FINGER_WIDTH*FINGER_HEIGHT*2+1];
typedef struct fingerprint_var {
    uint64_t        token;
    uint32_t        gid;
    uint32_t        fid;
    int32_t         cancel;
    pthread_t       tid;
    pthread_mutex_t fingerprint_lock;
    pthread_mutex_t count_lock;
    pthread_cond_t  count_nonzero;
} fingerprint_var_t;
//extern void btl_api_responseFunc(int __unused signum);

//extern fingerprint_var_t    gFingerprintVar;

void btl_nav_core_wakeNavThread(int __unused arg)
{
    LOGD(TAG, "%s", __func__);
    pthread_cond_signal(&gFingerprintVarNav.count_nonzero);
    //pthread_cond_signal(&gFingerprintVar.count_nonzero);
}
static void btl_nav_core_wakeNavCtlThread(int __unused arg)
{
    LOGD(TAG, "%s", __func__);
    pthread_cond_signal(&gFingerprintVarNavCtl.count_nonzero);
}
int32_t btl_nav_core_waitSignal(int32_t timeout_sec,int32_t timeout_ms)
{
    int ret;
    int Oflags;
    int counts = 0;
    int retWait = 0;
    ret = 0;

    LOGD(TAG, "++ %s", __func__);
    pthread_mutex_lock(&gFingerprintVarNav.count_lock);
    signal(SIGIO, &btl_nav_core_wakeNavThread);//btl_api_responseFunc//btl_nav_core_wakeNavThread
    int fd = open(deviceNode, O_RDWR);
    if(fd < 0) {
        LOGD(TAG, "can't open device");
        pthread_mutex_unlock(&gFingerprintVarNav.count_lock);
        return -1 ;
    }
    fcntl(fd, F_SETOWN, getpid());
    Oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, Oflags | FASYNC);

	if(timeout_sec == 0xff)
		pthread_cond_wait(&gFingerprintVarNav.count_nonzero,
						  &gFingerprintVarNav.count_lock);
	else {
		struct timespec timer;
		struct timeval now;
		memset(&timer, 0, sizeof(struct timespec));
		memset(&now, 0, sizeof(struct timeval));
		gettimeofday(&now, NULL);
		timer.tv_sec = now.tv_sec + timeout_sec;
		timer.tv_nsec = now.tv_usec * 1000+timeout_ms*1000*1000;
		retWait = pthread_cond_timedwait(&gFingerprintVarNav.count_nonzero,
										 &gFingerprintVarNav.count_lock,
										 &timer);
		if(retWait == ETIMEDOUT) {
			LOGD(TAG, "timeout");
			ret = -2;
		}
	}

    close(fd);
    pthread_mutex_unlock(&gFingerprintVarNav.count_lock);
    LOGD(TAG, "-- %s", __func__);
	return ret;

}

int read_navCtlPipe(char *buff)
{
     int  fd;
     int  nread;

     if (access(navCtrlPipe, F_OK) == -1)
     {

        if ((mkfifo(navCtrlPipe, 0666) < 0) /*&& (errno != EEXIST)*/)
        {
        	LOGE(TAG, "Cannot create nav ctl fifo file\n");
            return -1;
        }
        LOGD(TAG, "mkfifo nav ctl success\n");
      }
     LOGD(TAG, "access or mkfifo nav ctl success\n");

     fd = open(navCtrlPipe, O_RDONLY);
     LOGD(TAG, "Open nav ctl success\n");
     if (fd == -1)
     {
    	 LOGE(TAG, "Open nav ctl fifo file error\n");
    	 return -2;
     }
     memset(buff, 0, sizeof(buff));
     if ((nread = read(fd, buff, MAX_PIPEBUFFER_SIZE)) > 0)
     {

    	buff[nread-1] = '\0';
        LOGD(TAG, "Read len = %d: \"%s\" from nav ctl FIFO\n", nread, buff);
     }

     close(fd);
     return nread;
}
static int wirte_navCtlPipe(char *data)
{
	int fd;
	//char buff[MAX_PIPEBUFFER_SIZE];
	int nwrite;
	LOGD(TAG, "++ %s %s", __func__, data);
	if (access(navCtrlPipe, F_OK) == -1)
	{
		   LOGE(TAG, "nav ctl fifo file do not not exist\n");
		   if ((mkfifo(navCtrlPipe, 0666) < 0) /*&& (errno != EEXIST)*/)
        	{
	        	LOGE(TAG, "Cannot create nav ctl fifo file\n");
	            return -1;
        	}
        	LOGD(TAG, "mkfifo nav ctl success\n");
	 }
	//sscanf(data, "%s", buff);

	fd = open(navCtrlPipe, O_WRONLY);
	if (fd == -1)
	{
		LOGE(TAG,"Open fifo file error\n");
		return -1;
	}

	if ((nwrite = write(fd, data, strlen(data) + 1)) > 0)
	{
		LOGD(TAG,"Write '%s' to nav ctl FIFO\n", data);
		return -2;
	}
	close(fd);
	LOGD(TAG, "-- %s", __func__);
	return 0;
}
void btl_nav_getImageData(uint8_t *image, int contrast)
{
	int32_t tempFd;
	LOGD(TAG, "++ %s", __func__);
	tempFd = open(deviceNode,O_RDWR);
	//ioctl(tempFd,BL229X_CONTRAST_ADJUST, 170);
	ioctl(tempFd,BL229X_GET_IMAGEDATA, image);
	close(tempFd);
	LOGD(TAG, "-- %s", __func__);
}
int btl_nav_getBackLightSta(void)
{
	int32_t tempFd, ret = 1;
	char buf[5] = {0};
	LOGD(TAG, "++ %s", __func__);
	tempFd = open("sys/class/leds/lcd-backlight/brightness",O_RDWR);
	//tempFd = open("/sys/class/backlight/sprd_backlight/brightness",O_RDWR);
	
	//ioctl(tempFd,BL229X_CONTRAST_ADJUST, 170);
	read(tempFd, buf, sizeof(buf)/sizeof(buf[0]));

	close(tempFd);
	if(strncmp(buf, "0", 1) == 0)
	{
		LOGD(TAG, "lcd-backlight/brightness: %s ",  buf);
		ret = 0;
	}
	LOGD(TAG, "-- %s", __func__);
	return ret;

}
void simulate_key(int fd,int kval)  
{  
	int ret = 0;
    struct input_event event;  
    event.type = EV_KEY;  
    event.value = 1;  
    event.code = kval;  
  
    gettimeofday(&event.time,0);  
    if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD("-- %s %s %d", __func__, "write error", ret); 
  
    event.type = EV_SYN;  
    event.code = SYN_REPORT;  
    event.value = 0;  
    if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD("-- %s %s %d", __func__, "write error", ret); 
 
    memset(&event, 0, sizeof(event));  
    gettimeofday(&event.time, NULL);  
    event.type = EV_KEY;  
    event.code = kval;  
    event.value = 0;  
    if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD("-- %s %s %d", __func__, "write error", ret);
    event.type = EV_SYN;  
    event.code = SYN_REPORT;  
    event.value = 0;  
    if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD("-- %s %s %d", __func__, "write error", ret);

}  

void btl_nav_reportKey(int key)
{
	int32_t tempFd;
	int key_to;
	LOGD(TAG, "++ %s", __func__);

	tempFd = open(deviceNode,O_RDWR);
	//ioctl(tempFd,BL229X_CONTRAST_ADJUST, 170);
	if(key == CLK_LONG_DOWN)
		key_to = KEY_POWER;
	else if(key == CLK_SINGLE)
		key_to = KEY_BACK;//KEY_HOMEPAGE//KEY_BACK
	else if(key == CLK_DOUBLE)
		key_to = KEY_HOMEPAGE;
	ioctl(tempFd,BL229X_NAV_REPORT, &key_to);
	close(tempFd);
	LOGD(TAG, "-- %s", __func__);
}

void btl_nav_reportKey2(char key)
{
	int32_t tempFd;
	char key_to = key;
	LOGD(TAG, "++ %s", __func__);

	tempFd = open(eventDevNode,O_RDWR);
	if(tempFd < 0)
			LOGD(TAG, "%s open %s failed %d", __func__, eventDevNode, tempFd);
	if(key == CLK_LONG_DOWN)
		simulate_key(tempFd, KEY_HOMEPAGE);
	else if(key == CLK_SINGLE)
		simulate_key(tempFd, KEY_BACK);//KEY_HOMEPAGE//KEY_BACK
	else if(key == CLK_DOUBLE)
	simulate_key(tempFd, KEY_POWER);
#ifdef CHIP_BF3182

	#ifdef	NAV_ROTATION_180
		else if(key == DIR_RIGHT)
			simulate_key(tempFd, KEY_LEFT);
		else if(key == DIR_LEFT)
			simulate_key(tempFd, KEY_RIGHT);
	#else
		else if(key == DIR_RIGHT)
			simulate_key(tempFd, KEY_RIGHT);
		else if(key == DIR_LEFT)
			simulate_key(tempFd, KEY_LEFT);
	#endif

#else

	#ifdef	NAV_ROTATION_90
		else if(key == DIR_RIGHT)
			simulate_key(tempFd, KEY_UP);
		else if(key == DIR_LEFT)
			simulate_key(tempFd, KEY_DOWN);
		else if(key == DIR_UP)
			simulate_key(tempFd, KEY_LEFT);
		else if(key == DIR_DOWN)
			simulate_key(tempFd, KEY_RIGHT);
	#else
		else if(key == DIR_RIGHT)
			simulate_key(tempFd, KEY_RIGHT);
		else if(key == DIR_LEFT)
			simulate_key(tempFd, KEY_LEFT);
		else if(key == DIR_UP)
			simulate_key(tempFd, KEY_UP);
		else if(key == DIR_DOWN)
			simulate_key(tempFd, KEY_DOWN);
	#endif

#endif
	close(tempFd);
	LOGD(TAG, "-- %s", __func__);
}

void btl_pw_reportKey2(char key)
{
	int32_t tempFd;
	char key_to = key;
	LOGD("++ %s", __func__);
	
	LOGD(TAG, "%s eventDevNode: %s", __func__, eventDevNode);
	tempFd = open(eventDevNode,O_RDWR);
	if(tempFd < 0)
			LOGD("%s open %s failed %d", __func__, eventDevNode, tempFd);
	simulate_key(tempFd, KEY_HOMEPAGE);

	close(tempFd);
	LOGD("-- %s", __func__);
}

void simulate_Touch_X(int fd,int pos)  
{  
	int ret = 0, count = 0;
    struct input_event event;  
	//usleep(1000000);
	LOGD(TAG, "++ %s", __func__);
	memset(&event, 0, sizeof(event));
			
	event.type = EV_ABS;  
	event.code = ABS_MT_TRACKING_ID;  
	event.value = 154;
	
			
	if((ret = write(fd, &event, sizeof(event))) < 0)
		LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 


	event.type = EV_KEY;  
	event.code = BTN_TOUCH;  
	event.value = 1;
	
	if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
	if(pos)
	{
		event.type = EV_ABS;  
		event.code = ABS_MT_POSITION_X;  
		event.value = MOVE_X_STEP * count + EDGE_X_DIS;
		
		if((ret = write(fd, &event, sizeof(event))) < 0)
				LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
			
		event.type = EV_ABS;  
		event.code = ABS_MT_POSITION_Y;  
		event.value = Y_STABLE;
		
		if((ret = write(fd, &event, sizeof(event))) < 0)
				LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);

	}
	else
	{
		event.type = EV_ABS;  
		event.code = ABS_MT_POSITION_X;  
		event.value = ABS_X_MAX - EDGE_X_DIS;
		
		if((ret = write(fd, &event, sizeof(event))) < 0)
				LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
			
		event.type = EV_ABS;  
		event.code = ABS_MT_POSITION_Y;  
		event.value = Y_STABLE;
		
		if((ret = write(fd, &event, sizeof(event))) < 0)
				LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);

	}

	gettimeofday(&event.time,0);

	event.type = EV_SYN;  
	event.code = SYN_REPORT;  
	event.value = 0;  
	if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);		
				
 
	while(count < COUNT_MAX)
  	{
  		if(pos)
  		{			
			event.type = EV_ABS;  
			event.code = ABS_MT_POSITION_X;  
			event.value = MOVE_X_STEP * count + EDGE_X_DIS;

			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
				
			event.type = EV_ABS;  
			event.code = ABS_MT_POSITION_Y;  
			event.value = Y_STABLE;
			
			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);	
				
			gettimeofday(&event.time,0);
			event.type = EV_SYN;  
			event.code = SYN_REPORT;  
			event.value = 0;  
			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);
				
			
		}
		else
		{		
			event.type = EV_ABS;  
			event.code = ABS_MT_POSITION_X;  
			event.value = ABS_X_MAX - EDGE_X_DIS - MOVE_X_STEP * count;
			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
				
			event.type = EV_ABS;  
			event.code = ABS_MT_POSITION_Y;  
			event.value = Y_STABLE;
			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);	
			
			event.type = EV_SYN;  
			event.code = SYN_REPORT;  
			event.value = 0;  
			gettimeofday(&event.time,0);
			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);			
			
			LOGD(TAG, "-pos %s", __func__);

		}

		usleep(5000);
		count++;
	}
	event.type = EV_ABS;  
	event.code = ABS_MT_TRACKING_ID;  
	event.value = ~0;

	if((ret = write(fd, &event, sizeof(event))) < 0)
		LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
	
	event.type = EV_KEY;  
	event.code = BTN_TOUCH;  
	event.value = 0;
	if((ret = write(fd, &event, sizeof(event))) < 0)
		LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
	
	gettimeofday(&event.time,0);
	event.type = EV_SYN;  
	event.code = SYN_REPORT;  
	event.value = 0;  
	if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);	

	LOGD("-- %s", __func__);

}  

void simulate_Touch_Y(int fd,int pos)  
{  
	int ret = 0, count = 0;
    struct input_event event;  
	//usleep(1000000);
	LOGD(TAG, "++ %s", __func__);
	memset(&event, 0, sizeof(event));
			
	event.type = EV_ABS;  
	event.code = ABS_MT_TRACKING_ID;  
	event.value = 154;

			
	if((ret = write(fd, &event, sizeof(event))) < 0)
		LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 

	if(pos)
	{
		event.type = EV_ABS;  
		event.code = ABS_MT_POSITION_X;  
		event.value = X_STABLE;
	
		if((ret = write(fd, &event, sizeof(event))) < 0)
				LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
			
		event.type = EV_ABS;  
		event.code = ABS_MT_POSITION_Y;  
		event.value = MOVE_Y_STEP * count + EDGE_Y_DIS;
		if((ret = write(fd, &event, sizeof(event))) < 0)
				LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
	}
	else
	{
		event.type = EV_ABS;  
		event.code = ABS_MT_POSITION_X;  
		event.value = X_STABLE;
	
		if((ret = write(fd, &event, sizeof(event))) < 0)
				LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
			
		event.type = EV_ABS;  
		event.code = ABS_MT_POSITION_Y;  
		event.value = MOVE_Y_STEP * (COUNT_MAX - 1) + EDGE_Y_DIS;
		if((ret = write(fd, &event, sizeof(event))) < 0)
				LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
	}

	event.type = EV_KEY;  
	event.code = BTN_TOUCH;  
	event.value = 1;
	
	if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
	
	gettimeofday(&event.time,0);

	event.type = EV_SYN;  
	event.code = SYN_REPORT;  
	event.value = 0;  
	if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);		
			
	while(count < COUNT_MAX)
  	{
  		if(pos)
  		{
			event.type = EV_ABS;  
			event.code = ABS_MT_POSITION_X;  
			event.value = X_STABLE;

			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);	
				
			event.type = EV_ABS;  
			event.code = ABS_MT_POSITION_Y;  
			event.value = MOVE_Y_STEP * count + EDGE_Y_DIS;
			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
				
			gettimeofday(&event.time,0);
			event.type = EV_SYN;  
			event.code = SYN_REPORT;  
			event.value = 0;  
			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);
		}
		else
		{					
			event.type = EV_ABS;  
			event.code = ABS_MT_POSITION_X;  
			event.value = X_STABLE;

			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
				
			event.type = EV_ABS;  
			event.code = ABS_MT_POSITION_Y;  
			event.value = MOVE_Y_STEP * (COUNT_MAX - count - 1) + EDGE_Y_DIS;

			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);	
			
			event.type = EV_SYN;  
			event.code = SYN_REPORT;  
			event.value = 0;  
			gettimeofday(&event.time,0);
			if((ret = write(fd, &event, sizeof(event))) < 0)
					LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);			
					
		}

		usleep(5000);
		count++;
	}

	event.type = EV_ABS;  
	event.code = ABS_MT_TRACKING_ID;  
	event.value = ~0;

	if((ret = write(fd, &event, sizeof(event))) < 0)
		LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
	
	event.type = EV_KEY;  
	event.code = BTN_TOUCH;  
	event.value = 0;
	if((ret = write(fd, &event, sizeof(event))) < 0)
		LOGD(TAG, "-- %s %s %d", __func__, "write error", ret); 
	
	gettimeofday(&event.time,0);
	event.type = EV_SYN;  
	event.code = SYN_REPORT;  
	event.value = 0;  
	if((ret = write(fd, &event, sizeof(event))) < 0)
			LOGD(TAG, "-- %s %s %d", __func__, "write error", ret);	
	LOGD(TAG,"-- %s", __func__);

}  
int btl_nav_simulateTouch(char *touch)
{
	int32_t tempFd;
	int key = 0;
	LOGD(TAG, "++ %s", __func__);
	LOGD(TAG, "%s eventDevNode: %s", __func__, eventDevNode);
	tempFd = open(eventDevNode, O_RDWR);
	//ioctl(tempFd,BL229X_CONTRAST_ADJUST, 170);
	if(strncmp(touch, "left", 4) == 0)
	{
		LOGD(TAG, "left %s", __func__);
		simulate_Touch_X(tempFd, 0);		
	}	

	else if(strncmp(touch, "right", 5) == 0)
	{
		LOGD(TAG, "right %s", __func__);
		simulate_Touch_X(tempFd, 1);		
	}else if(strncmp(touch, "up", 2) == 0)
	{
		LOGD(TAG, "left %s", __func__);
		simulate_Touch_Y(tempFd, 0);		
	}	
	else if(strncmp(touch, "down", 4) == 0)
	{
		LOGD(TAG, "right %s", __func__);
		simulate_Touch_Y(tempFd, 1);		
	}
	close(tempFd);
	LOGD(TAG, "-- %s", __func__);
	return 0;
}


int btl_nav_LaucherNav(char key)
{
	switch (key)
	{
#ifdef CHIP_BF3182

	#ifdef NAV_ROTATION_180
		case DIR_RIGHT:
			btl_nav_simulateTouch("up");
			break;
		case DIR_LEFT:
			btl_nav_simulateTouch("down");
			break;
	#else
		case DIR_RIGHT:
			btl_nav_simulateTouch("down");
			break;
		case DIR_LEFT:
			btl_nav_simulateTouch("up");
			break;
	#endif

#else

	#ifdef NAV_ROTATION_90
		case DIR_RIGHT:
			btl_nav_simulateTouch("up");
			break;
		case DIR_LEFT:
			btl_nav_simulateTouch("down");
			break;
		case DIR_DOWN:
			btl_nav_simulateTouch("right");
			break;
		case DIR_UP:
			btl_nav_simulateTouch("left");

	#else
		case DIR_RIGHT:
			btl_nav_simulateTouch("right");
			break;
		case DIR_LEFT:
			btl_nav_simulateTouch("left");
			break;
		case DIR_DOWN:
			btl_nav_simulateTouch("up");
			break;
		case DIR_UP:
			btl_nav_simulateTouch("down");
			break;		
	#endif
#endif	
			default:
			LOGD(TAG, "no simulate Touch");
			break;	
	}
	return 0;
}

int btl_nav_func(char key)
{
	switch(key)
	{
	case DIR_RIGHT:
		nav_dir_func.right == NAV_TOUCH_ACTOIN ? btl_nav_LaucherNav(key) : btl_nav_reportKey2(key);
		break;
	case DIR_LEFT:
		nav_dir_func.left == NAV_TOUCH_ACTOIN ? btl_nav_LaucherNav(key) : btl_nav_reportKey2(key);
		break;
	case DIR_DOWN:
		nav_dir_func.down == NAV_TOUCH_ACTOIN ? btl_nav_LaucherNav(key) : btl_nav_reportKey2(key);
		break;
	case DIR_UP:
		nav_dir_func.up == NAV_TOUCH_ACTOIN ? btl_nav_LaucherNav(key) : btl_nav_reportKey2(key);
		break;
	default:
		break;
	}
	
	return 0;
	
}

static void btl_nav_core_setCancel(int arg)
{
	gFingerprintVarNav.cancel = arg;
}
static void btl_navCtl_core_setCancel(int arg)
{
	gFingerprintVarNavCtl.cancel = arg;
}
int btl_nav_core_start(void)
{
	LOGD(TAG, "++ %s", __func__);
	btl_nav_core_setCancel(0);
	btl_nav_core_wakeNavThread(1);
	btl_navCtl_core_setCancel(0);
	//btl_nav_core_wakeNavCtlThread(1);
	LOGD(TAG, "-- %s", __func__);
	return 0;
}

int btl_nav_core_stop(void)
{
	LOGD(TAG, "++ %s", __func__);
	btl_nav_core_setCancel(1);
	//btl_navCtl_core_setCancel(1);
	btl_nav_core_wakeNavThread(1);
	LOGD(TAG, "-- %s", __func__);
	return 0;
}

int btl_nav_core_cancel(void)
{
	LOGD(TAG, "++ %s", __func__);
	btl_nav_core_setCancel(2);
	//btl_navCtl_core_setCancel(1);
	btl_nav_core_wakeNavThread(1);
	LOGD(TAG, "-- %s", __func__);
	return 0;
}
static void *thNavigationReportFunc(void *arg)
{
	int ret = 0;
	pthread_detach(pthread_self());
	pthread_mutex_lock(&gFingerprintVarNavReport.fingerprint_lock);
	char buff[MAX_PIPEBUFFER_SIZE] = {' '};
	inNavReporting = 1;
	LOGD(TAG, "++%s", __func__);
	usleep(gFingerprintParamNavReport.timeout_sec * 1000);
	if(gFingerprintVarNavReport.cancel == 0)
	{
		LOGD(TAG, "thNavigationReportFunc Running ...");
		//sleep(1);
		btl_nav_reportKey(gFingerprintParamNavReport.operation_id);
//		btl_nav_reportKey2(gFingerprintParamNavReport.operation_id);
		gFingerprintParamNavReport.operation_id = 0;
		//wirte_navCtlPipe("RDY");
	}
	
	gFingerprintParamNavReport.timeout_sec = 0;
	gFingerprintParamNavReport.operation_id = 0;
	gFingerprintVarNavReport.cancel = 0;

	inNavReporting = 0;
	pthread_mutex_unlock(&gFingerprintVarNavReport.fingerprint_lock);
    LOGD(TAG, "--%s", __func__);
    return ((void *)0);
}

static void *thNavigationFunc(void *arg)
{
	static struct timeval before, now, diff;
	unsigned long interval_time = 0;
	pthread_detach(pthread_self());
	//gFingerprintVarNav.tid = pthread_self();
	pthread_mutex_lock(&gFingerprintVarNav.fingerprint_lock);
	char key = 0, tmp = 1, ret = 0;
	gettimeofday(&now,0);
	while(1)
	{
		//LOGD(TAG, "in Nav !");
		if(gFingerprintVarNav.cancel == 0)
		{
			LOGD(TAG, "Nav Running ...");
			if(tmp == 1)
			{
				tmp = 0;
				usleep(10000);
			}
			
			btl_nav_core_waitSignal(0xff, 0);
			if(gFingerprintVarNav.cancel == 0 && btl_nav_getBackLightSta() == 1)
			{
				memcpy(&before, &now, sizeof(struct timeval));
				gettimeofday(&now,0);
				timeval_subtract(&diff,&before,&now);
				key = cal_touch_area();
				
#ifdef NAV_DIR_EN	
				if(key ==DIR_RIGHT || key ==DIR_LEFT || key ==DIR_UP || key ==DIR_DOWN )
				{
					btl_nav_func(key);
					continue;
				}
#endif

#ifdef NAV_LONGCLK_EN
				if(key ==CLK_LONG_DOWN)
				{
					btl_nav_reportKey(key);
					//btl_nav_reportKey2(key);
					continue;					
				}
#endif

#ifdef NAV_SINGLECLK_EN
				if(key == CLK_SINGLE || key == ACT_NONE)
				{
					//btl_nav_reportKey2(key);
					if(key == CLK_SINGLE)
					{
						gFingerprintParamNavReport.timeout_sec = 300;
						gFingerprintVarNavReport.cancel = 0;
						gFingerprintParamNavReport.operation_id = CLK_SINGLE;
						if(inNavReporting == 0)
						{
							ret = pthread_create(&gFingerprintVarNavReport.tid, NULL, &thNavigationReportFunc, &gFingerprintParamNavReport);
							if(ret != 0) {
								LOGE(TAG, "create Navigation Report thread error: %s/n", strerror(ret));
								continue;
							}
						}
					}
					
					interval_time = diff.tv_sec * 1000 + diff.tv_usec/1000;
					LOGD(TAG, "CLK_SINGLE diff interval_time %ld", interval_time);
					if(interval_time < 300)
						key = CLK_DOUBLE;
				}
#endif

#ifdef NAV_DOUBLECLK_EN
				if(key == CLK_DOUBLE)
				{
					LOGD(TAG, "CLK_DOUBLE");
					gFingerprintVarNavReport.cancel = 1;
					usleep(100000);
					btl_nav_reportKey(key);
//					btl_nav_reportKey2(key);
					continue;					
				}
#endif


			}
		}
		else if(gFingerprintVarNav.cancel == 1)
		{
			LOGD(TAG, "thNavigationFunc Stop ++");
			pthread_mutex_lock(&gFingerprintVarNav.count_lock);
			pthread_cond_wait(&gFingerprintVarNav.count_nonzero, &gFingerprintVarNav.count_lock);
			pthread_mutex_unlock(&gFingerprintVarNav.count_lock);
			tmp = 1;
			
			LOGD(TAG, "thNavigationFunc Stop --  cancel = %d", gFingerprintVarNav.cancel);
		}
		else if(gFingerprintVarNav.cancel == 2)
		{
			LOGD(TAG, "thNavigationFunc CANCEL ...");
			gFingerprintVarNav.tid = 0;
			wirte_navCtlPipe("CCL");
			break;
		}
	}
	pthread_mutex_unlock(&gFingerprintVarNav.fingerprint_lock);
    LOGD(TAG, "--%s", __func__);
    return ((void *)0);
}

static void *thNavigationCtlFunc(void *arg)
{
	int ret = 0;
	pthread_detach(pthread_self());
	pthread_mutex_lock(&gFingerprintVarNavCtl.fingerprint_lock);
	char buff[MAX_PIPEBUFFER_SIZE] = {' '};
	while(1)
	{
		if(gFingerprintVarNavCtl.cancel == 0)
		{
			LOGD(TAG, "thNavigationCtlFunc Running ...");
			//sleep(1);

			ret = read_navCtlPipe(buff);


			if(!memcmp(buff, "STP", ret))
			{
				btl_nav_core_setCancel(1);
				btl_nav_core_wakeNavThread(1);
			}
			else if(!memcmp(buff, "STT", ret))
			{
				usleep(1500000);
				btl_nav_core_start();
			}
			else if(!memcmp(buff, "CNL", ret))
			{
				if(gFingerprintVarNav.tid != 0)
				{
					btl_nav_core_setCancel(2);
					btl_nav_core_wakeNavThread(1);
				}
			}
			else if(!memcmp(buff, "CCL", ret))
			{
				if(gFingerprintVarNavCtl.tid != 0)
				{
					btl_navCtl_core_setCancel(2);
				}
			}
			
			//wirte_navCtlPipe("RDY");
		}
		else if(gFingerprintVarNavCtl.cancel == 1)
		{
			LOGD(TAG, "thNavigationCtlFunc Stop ...");
			pthread_mutex_lock(&gFingerprintVarNavCtl.count_lock);
			pthread_cond_wait(&gFingerprintVarNavCtl.count_nonzero, &gFingerprintVarNavCtl.count_lock);
			pthread_mutex_unlock(&gFingerprintVarNavCtl.count_lock);
		}
		else if(gFingerprintVarNavCtl.cancel == 2)
		{
			LOGD(TAG, "thNavigationCtlFunc CANCEL ...");
			gFingerprintVarNavCtl.tid = 0;
			break;
		}
	}
	pthread_mutex_unlock(&gFingerprintVarNavCtl.fingerprint_lock);

    LOGD(TAG, "--%s", __func__);
    return ((void *)0);
}

int btl_nav_core_init(void)
{
	LOGD(TAG, "++ %s %d", __func__, gFingerprintVarNav.tid);
	int err = 0;
	gFingerprintVarNav.cancel = 0;
//	if(gFingerprintVarNav.tid == 0)
	{
		err = pthread_create(&gFingerprintVarNav.tid, NULL, &thNavigationFunc, &gFingerprintParamNav);
		if(err != 0) {
			LOGE(TAG, "create Navigation thread error: %s/n", strerror(err));
			return -1;
		}
	}

    LOGD(TAG, "--%s", __func__);
	return 0;

}
int btl_nav_core_ctl_getSta(void)
{
	char nav_sta[100] = {'\0'};
	int ret = 0;
	LOGD(TAG, "++ %s", __func__);
	ret = property_get("persist.blestech.nav", nav_sta, NULL);
	LOGD(TAG, "ret = %d, persist.blestech.nav %s", ret, nav_sta);
	if(strncmp(nav_sta, "enable", 6) == 0)
		ret = 1;
	else if(strncmp(nav_sta, "disable", 7) == 0)
		ret = 0;
	else
		ret = -1;
	LOGD(TAG, "%s : %s, ret = %d", __func__, nav_sta, ret);
	
	LOGD(TAG, "--%s", __func__);
	return ret;
}

int btl_nav_core_ctl_setSta(int sta)
{
	int ret = 0;
	LOGD(TAG, "++ %s", __func__);
	if(sta)
		ret = property_set("persist.blestech.nav", "enable");
	else
		ret = property_set("persist.blestech.nav", "disable");

	LOGD(TAG, "%s : %d, ret = %d", __func__, sta, ret);
	LOGD(TAG, "--%s", __func__);
	return ret;
}

int btl_nav_core_ctl_uinit(void)
{
	LOGD(TAG, "++ %s", __func__);
//	if(gFingerprintVarNavCtl.tid != 0)
	{
		wirte_navCtlPipe("CNL");
//		btl_navCtl_core_setCancel(2);
//		btl_nav_core_wakeNavCtlThread(1);
		
	}
	LOGD(TAG, "--%s", __func__);
	return 0;
}
int btl_nav_core_ctl_init(void)
{
	int err = 0;

	LOGD(TAG, "++ %s", __func__);
	
	gFingerprintVarNavCtl.cancel = 0;
	if(gFingerprintVarNavCtl.tid == 0)
		{
			err = pthread_create(&gFingerprintVarNavCtl.tid, NULL, &thNavigationCtlFunc, &gFingerprintParamNavCtl);
				if(err != 0) {
					LOGE(TAG, "create NavigationCtl thread error: %s/n", strerror(err));
					return -1;
			}
		}

    LOGD(TAG, "--%s", __func__);
	return 0;

}
