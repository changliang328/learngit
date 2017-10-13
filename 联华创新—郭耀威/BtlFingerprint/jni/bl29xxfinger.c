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
#include <pthread.h>

#include <jni.h>
#include <android/log.h>

#define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,  "btl_jni",__VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG  ,  "btl_jni",__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO   ,  "btl_jni",__VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN   ,  "btl_jni",__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR  ,  "btl_jni",__VA_ARGS__)

#define BL229X_IOCTL_MAGIC_NO           0xFC
#define BL229X_INIT                       _IO(BL229X_IOCTL_MAGIC_NO,  0)
#define BL229X_GETIMAGE    		       _IOW(BL229X_IOCTL_MAGIC_NO, 1, uint32_t)
#define BL229X_INTERRUPT_MODE           _IOW(BL229X_IOCTL_MAGIC_NO, 2, uint32_t)
#define BL229X_Adjustment_AGC           _IOW(BL229X_IOCTL_MAGIC_NO, 3, uint8_t )
#define BL229X_POWERDOWN_MODE           _IO(BL229X_IOCTL_MAGIC_NO,  4          )
#define BL229X_MULTIFUNCTIONAL_KEYCODE	_IOW(BL229X_IOCTL_MAGIC_NO, 6, uint32_t) 
#define BL229X_GET_ID	                _IOWR(BL229X_IOCTL_MAGIC_NO, 9, uint32_t)

#define MAX_FINGER_NUMS    5
#define TIME_OVER                                   3
#define MAX_AGC_ADJUST_COUNT						6
#define INTENSITY_LIMIT								5
#define SENSOR_CONTRAST_LOW_LIMIT	(0x0)
#define SENSOR_CONTRAST_HIGH_LIMIT	(0xF8)
#define SENSOR_DEFAULT_CONTRAST (0x50)
#define RAW_FINGERPRINT_SIZE	(112*96)

#define caparam1            65536
#define caparam2            13
#define RIGHT_SHIFTBIT(val,nbits)        (val>>nbits)
#define caparam3            39567
#define caparam4            65536
#define AGC_FOR_TEST_EMPTY	(0x50)

#define EMPTY_PIXCEL_VALUE (0xff)//0xff
//取消读取的标记
int mCancelFlag = 0;
//图像块灰度均值
//uint32_t M_block[168]= {0};
//图像块绝对偏差
//uint32_t D_block[168]= {0};

uint8_t flag = 0;
int g_nContrast = SENSOR_DEFAULT_CONTRAST; //0xfc; //0xf8
uint16_t best_agc = 0x80;
int fd = -1;
uint8_t wg_flag_anya= 0;
int dry_flag = 0;
uint8_t mDry = 0;
uint8_t mIntensity = 0;
uint8_t mContrast = 0;

int count_failed = 0;
int count_success = 0;

int mSize = 0;
#define IMAGE_WIDTH 112
#define IMAGE_HEIGHT 96

char* deviceFolderPath =   "/dev/bl229x";

#define FINGER_DEBUG

extern int32_t btl_core_isFingerUp();

int User_PowerDown()
{
#ifdef FINGER_DEBUG
    LOGD("User_PowerDown begin: enter PowerDown mode");
#endif
	int tempFd = -1;
    tempFd = open(deviceFolderPath,O_RDWR);
    if (tempFd == -1) {
        LOGE ("can't open BL229X!\n");
        return -1;
    }
	fd = tempFd;
    /* 进入PowerDown模式 */
    ioctl(fd,BL229X_POWERDOWN_MODE);
    close(fd);
    return 0;
}
int User_WriteKeycode(int keycode)
{
#ifdef FINGER_DEBUG
    LOGD("User_WriteKeycode begin: enter User_WriteKeycode mode");
#endif

	int tempFd = -1;
    tempFd = open(deviceFolderPath,O_RDWR);
    if (tempFd == -1) {
        LOGE ("can't open BL229X!\n");
        return -1;
    }
	fd = tempFd;

    ioctl(fd,BL229X_MULTIFUNCTIONAL_KEYCODE,keycode);
    close(fd);
    return 0;
}

int User_WaitScreenOn()
{
    int tempFd = -1;
    tempFd = open(deviceFolderPath,O_RDWR);
    if (tempFd == -1) {
        LOGE ("can't open BL229X!\n");
        return -1;
    }
	fd = tempFd;

    ioctl(fd,BL229X_INTERRUPT_MODE);//进入中断模式
    close(fd);
    return 0;
}

int User_GetId(int *outbuffer)
{
    uint32_t buffer[20];
	int tempfd;
    tempfd = open(deviceFolderPath,O_RDWR);
    if (tempfd == -1) {
        LOGE ("can't open bl229x!\n");
        return -1;
    }

    ioctl(tempfd,BL229X_GET_ID,buffer);//杩涘叆涓柇妯″紡
    close(tempfd);
    LOGW ("%x,%x",buffer[0],buffer[1]);
    *outbuffer  =  buffer[0];
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////手指抬起判断函数
int User_IsFingerUp ()
{
    return btl_core_isFingerUp();
}
