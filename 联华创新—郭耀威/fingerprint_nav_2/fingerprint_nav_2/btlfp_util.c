/*
 * Copyright (C) 2016 BetterLife.Co.Ltd. All rights  reserved.
 *
 */

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

#include <android/log.h>




#define TAG       "btl_algo"
#define LOGV(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_VERBOSE, TAG,__VA_ARGS__);}
#define LOGD(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_DEBUG  , TAG,__VA_ARGS__);}
#define LOGI(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_INFO   , TAG,__VA_ARGS__);}
#define LOGW(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_WARN   , TAG,__VA_ARGS__);}
#define LOGE(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_ERROR  , TAG,__VA_ARGS__);}

uint8_t g_debug_enable;

struct timespec startTime1 = {0,0};
struct timespec startTime2 = {0,0};
struct timespec startTime3 = {0,0};


#ifdef simulate_touch
#ifdef newBundle
const char *input_dev = "/dev/input/event6";
#else
const char *input_dev = "/dev/input/event0";
#endif
#endif


void btl_util_time_update(uint8_t id)
{
    if (g_debug_enable) {
        if (id == 1)
            clock_gettime(CLOCK_MONOTONIC, &startTime1);
        else if (id == 2)
            clock_gettime(CLOCK_MONOTONIC, &startTime2);
        else if (id == 3)
            clock_gettime(CLOCK_MONOTONIC, &startTime3);
    }
}

void btl_util_time_diffnow(char *pTag, uint8_t id)
{
    struct timespec temp;
    struct timespec startTime,endTime;
    long   sec,msec;

    if (!g_debug_enable)
        return ;

    if (id < 1 || id > 3)
        return;

    if (id == 1) {
        startTime.tv_sec  = startTime1.tv_sec;
        startTime.tv_nsec = startTime1.tv_nsec;
    } else if (id == 2) {
        startTime.tv_sec  = startTime2.tv_sec;
        startTime.tv_nsec = startTime2.tv_nsec;
    } else if (id == 3) {
        startTime.tv_sec  = startTime3.tv_sec;
        startTime.tv_nsec = startTime3.tv_nsec;
    }

    clock_gettime(CLOCK_MONOTONIC, &endTime);
    if ((endTime.tv_nsec - startTime.tv_nsec) < 0) {
        temp.tv_sec = endTime.tv_sec - startTime.tv_sec - 1;
        temp.tv_nsec = 1000000000 + endTime.tv_nsec - startTime.tv_nsec;
    } else {
        temp.tv_sec = endTime.tv_sec - startTime.tv_sec;
        temp.tv_nsec = endTime.tv_nsec - startTime.tv_nsec;
    }
    sec = temp.tv_sec;
    msec = temp.tv_nsec / (1000*1000);
    LOGD("%s,%s, id:%d, diff time: %ld s,%ld ms,%ld",__func__,pTag,id,sec,msec,temp.tv_nsec);
}


// from <linux/input.h>
#ifdef simulate_touch
struct input_event {
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};

#define EVIOCGVERSION		_IOR('E', 0x01, int)			/* get driver version */
#define EVIOCGID		    _IOR('E', 0x02, struct input_id)	/* get device ID */
#define EVIOCGKEYCODE		_IOR('E', 0x04, int[2])			/* get keycode */
#define EVIOCSKEYCODE		_IOW('E', 0x04, int[2])			/* set keycode */

#define EVIOCGNAME(len)		_IOC(_IOC_READ, 'E', 0x06, len)		/* get device name */
#define EVIOCGPHYS(len)		_IOC(_IOC_READ, 'E', 0x07, len)		/* get physical location */
#define EVIOCGUNIQ(len)		_IOC(_IOC_READ, 'E', 0x08, len)		/* get unique identifier */

#define EVIOCGKEY(len)		_IOC(_IOC_READ, 'E', 0x18, len)		/* get global keystate */
#define EVIOCGLED(len)		_IOC(_IOC_READ, 'E', 0x19, len)		/* get all LEDs */
#define EVIOCGSND(len)		_IOC(_IOC_READ, 'E', 0x1a, len)		/* get all sounds status */
#define EVIOCGSW(len)		_IOC(_IOC_READ, 'E', 0x1b, len)		/* get all switch states */

#define EVIOCGBIT(ev,len)	_IOC(_IOC_READ, 'E', 0x20 + ev, len)	/* get event bits */
#define EVIOCGABS(abs)		_IOR('E', 0x40 + abs, struct input_absinfo)		/* get abs value/limits */
#define EVIOCSABS(abs)		_IOW('E', 0xc0 + abs, struct input_absinfo)		/* set abs value/limits */

#define EVIOCSFF		     _IOC(_IOC_WRITE, 'E', 0x80, sizeof(struct ff_effect))	/* send a force effect to a force feedback device */
#define EVIOCRMFF		     _IOW('E', 0x81, int)			/* Erase a force effect */
#define EVIOCGEFFECTS		 _IOR('E', 0x84, int)			/* Report number of effects playable at the same time */

#define EVIOCGRAB		     _IOW('E', 0x90, int)			/* Grab/Release device */

// end <linux/input.h>


int btl_util_sendevent(const char* arg1,char* arg2, char* arg3,char*arg4)
{
    int fd;
    ssize_t ret;
    int version;
    struct input_event event;


    fd = open(arg1, O_RDWR);
    if(fd < 0) {
        //fprintf(stderr, "could not open %s, %s\n", argv[optind], strerror(errno));
        return 1;
    }
    if (ioctl(fd, EVIOCGVERSION, &version)) {
        //fprintf(stderr, "could not get driver version for %s, %s\n", argv[optind], strerror(errno));
        return 1;
    }
    memset(&event, 0, sizeof(event));
    event.type = atoi(arg2);
    event.code = atoi(arg3);
    event.value = atoi(arg4);
    ret = write(fd, &event, sizeof(event));
    if(ret < (ssize_t) sizeof(event)) {
        //fprintf(stderr, "write event failed, %s\n", strerror(errno));
        return -1;
    }
    return 0;
}
#endif

int btl_util_simultetouch()
{
#ifdef simulate_touch
    LOGD("%s, start send event", __func__);
#ifndef newBundle
    btl_util_sendevent(input_dev,"3","57","256");
    btl_util_sendevent(input_dev,"3","53","345");
    btl_util_sendevent(input_dev,"3","54","546");
    btl_util_sendevent(input_dev,"1","330","1");
    btl_util_sendevent(input_dev,"0","0","0");
    btl_util_sendevent(input_dev,"3","57","-1");
    btl_util_sendevent(input_dev,"1","330","0");
    btl_util_sendevent(input_dev,"0","0","0");
#else
    btl_util_sendevent(input_dev,"3","57","0");   // EV_ABS    ABS_MT_TRACKING_ID   00000000
    btl_util_sendevent(input_dev,"1","330","1");  // EV_KEY    BTN_TOUCH            DOWN
    btl_util_sendevent(input_dev,"3","48","42");  // EV_ABS    ABS_MT_TOUCH_MAJOR   0000002a
    btl_util_sendevent(input_dev,"3","53","763"); // EV_ABS    ABS_MT_POSITION_X    00000207
    btl_util_sendevent(input_dev,"3","54","352"); // EV_ABS    ABS_MT_POSITION_Y    000000f0
    btl_util_sendevent(input_dev,"0","2","0");    // EV_SYN    SYN_MT_REPORT        00000000
    btl_util_sendevent(input_dev,"0","0","0");    // EV_SYN    SYN_REPORT           00000000

    btl_util_sendevent(input_dev,"3","57","0");   // EV_ABS    ABS_MT_TRACKING_ID   00000000
    btl_util_sendevent(input_dev,"3","48","42");  // EV_ABS    ABS_MT_TOUCH_MAJOR   0000002a
    btl_util_sendevent(input_dev,"3","53","763"); // EV_ABS    ABS_MT_POSITION_X    00000207
    btl_util_sendevent(input_dev,"3","54","352"); // EV_ABS    ABS_MT_POSITION_Y    000000f0
    btl_util_sendevent(input_dev,"0","2","0");    // EV_SYN    SYN_MT_REPORT        00000000
    btl_util_sendevent(input_dev,"0","0","0");    // EV_SYN    SYN_REPORT           00000000

    btl_util_sendevent(input_dev,"0","2","0");    // EV_SYN    SYN_MT_REPORT        00000000
    btl_util_sendevent(input_dev,"1","330","0");  // EV_KEY    BTN_TOUCH            UP
    btl_util_sendevent(input_dev,"0","0","0");    // EV_SYN    SYN_REPORT           00000000
#endif
#endif
    return 0;
}



uint32_t btl_util_my_hton(uint32_t ip)
{
    /* &ip指向用32个bit表示的整型数ip，将int*类型的指针转换为char*类型的指针，
     * 则ptr指向用8个bit表示(第0-7位)的整型数，
     * ptr+1指向用8个bit表示(第8-15位)的整型数，
     * ptr+2指向用8个bit表示(第16-23位)的整型数，
     * ptr+3指向用8个bit表示(第24-31位)的整型数。*/


    char *ptr = (char*)&ip;

    char tmp;
    tmp = ptr[0];
    ptr[0] = ptr[3];
    ptr[3] = tmp;

    tmp = ptr[1];
    ptr[1] = ptr[2];
    ptr[2] = tmp;

    return ip;
}

uint64_t btl_util_my_htonl(uint64_t ip)
{
    char *ptr = (char*)&ip;
    char tmp = 0;
    int i = 0;

    for(i=0; i<4; i++) {
        tmp = ptr[i];
        ptr[i] = ptr[7-i];
        ptr[7-i] = tmp;
    }

    return ip;
}



