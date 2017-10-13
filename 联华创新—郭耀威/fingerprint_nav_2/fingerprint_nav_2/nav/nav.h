/******************** (C) COPYRIGHT 2017 BTL ********************************
* File Name          : nav.h
* Author               : guoyaowei
* Version              : 1.0
* Date                  : 2017.2.21
*******************************************************************************/

#ifndef __NAV_H__
#define __NAV_H__

#include <pthread.h>
#include <sys/ioctl.h>

#include "../btlcustom.h"

#define INACTIVE_PXL	200

#if 1
#define ACT_NONE		0
#define DIR_RIGHT		3	//
#define DIR_LEFT		4
#define DIR_DOWN		2	//
#define DIR_UP			1   //
#define CLK_SINGLE		5
#define CLK_DOUBLE		6
#define CLK_LONG_DOWN	7
#define CLK_LONG_UP		8

#else
#define ACT_NONE		0
#define DIR_RIGHT		1
#define DIR_LEFT		2
#define DIR_DOWN		3
#define DIR_UP			4
#define CLK_SINGLE		5
#define CLK_DOUBLE		6
#define CLK_LONG_DOWN	7
#define CLK_LONG_UP		8
#endif


//========customization=======================

#define	NAV_SINGLECLK_EN	1
#define	NAV_LONGCLK_EN		1
//#define	NAV_DIR_EN			1
#define NAV_DOUBLECLK_EN	1

#ifdef CHIP_BF3182
	#define	NAV_ROTATION_180	1
#else
//	#define	NAV_ROTATION_90		1
#endif
//===============================

typedef enum NAV_ACTION
{
	NAV_TOUCH_ACTOIN	= 0,
	NAV_KEY_ACTOIN	= 1
}NAV_ACTION_T;

typedef struct NAV_FUNCH_DEF {
    NAV_ACTION_T right;
    NAV_ACTION_T left;
    NAV_ACTION_T up;
    NAV_ACTION_T down;
} NAV_FUNCH_DEF_T;


static NAV_FUNCH_DEF_T nav_dir_func = {NAV_TOUCH_ACTOIN, NAV_TOUCH_ACTOIN, NAV_TOUCH_ACTOIN, NAV_TOUCH_ACTOIN};

#define USE_PIPE_CONN

#define COUNT_MAX		20

#define	EDGE_X_DIS		ABS_X_MAX/5
//#define	EDGE_Y_DIS		ABS_Y_MAX/5
#define	EDGE_Y_DIS		0

#define MOVE_X_STEP	(ABS_X_MAX-EDGE_X_DIS*2)/COUNT_MAX
#define X_STABLE	ABS_X_MAX/2

#define MOVE_Y_STEP	(ABS_Y_MAX/2-EDGE_Y_DIS*2)/COUNT_MAX
#define Y_STABLE	ABS_Y_MAX/2

typedef struct fingerprint_param_nav {
    uint32_t gid;
    uint32_t fid;
    uint32_t timeout_sec;
    uint64_t operation_id;
} fingerprint_param_nav_t;

typedef struct fingerprint_var_nav {
    uint64_t        token;
    uint32_t        gid;
    uint32_t        fid;
    int32_t         cancel;
    pthread_t       tid;
    pthread_mutex_t fingerprint_lock;
    pthread_mutex_t count_lock;
    pthread_cond_t  count_nonzero;
} fingerprint_var_nav_t;

#define BL229X_IOCTL_MAGIC_NO         0xFC
#define BL229X_GET_IMAGEDATA	        _IOWR(BL229X_IOCTL_MAGIC_NO, 15, uint32_t)
#define BL229X_CONTRAST_ADJUST          _IOW(BL229X_IOCTL_MAGIC_NO, 3, uint32_t)
#define BL229X_NAV_REPORT          _IOWR(BL229X_IOCTL_MAGIC_NO, 16, uint32_t)

int32_t btl_nav_core_waitSignal(int32_t timeout_sec,int32_t timeout_ms);
void btl_nav_getImageData(uint8_t *image, int contrast);
void btl_nav_reportKey(int key);
static const char*  deviceNode  = "/dev/bl229x";
static const char*  eventDevNode  = "/dev/input/event1";

int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y);

int btl_nav_core_start(void);
int btl_nav_core_stop(void);
int btl_nav_core_init(void);
int btl_nav_core_uinit(void);

int btl_nav_core_cancel(void);

char cal_touch_area(void);
int btl_nav_core_ctl_init(void);
int btl_nav_core_ctl_uinit(void);
int btl_nav_core_ctl_getSta(void);
int btl_nav_core_ctl_setSta(int sta);
int btl_nav_simulateTouch(char *touch);

#define MAX_PIPEBUFFER_SIZE		1024
static const char *navCtrlPipe = "/data/system/users/0/fpdata//navCtrlPipe";
//static const char *navCtrlPipe = "/data/user/0/com.blestech.navigation/navCtrlPipe";
//static const char *navCtrlPipe = "/data/user/0/com.btlfinger.fingerprintunlock/navCtrlPipe";
#endif	/* __NAV_H__ */
