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
#include <unistd.h>
#include <math.h>
#include <android/log.h>

#include "btlfp.h"


extern uint8_t g_debug_enable;

#define OLD_NODE

#ifdef OLD_NODE
const char*  deviceNode  = "/dev/bl229x";
#else 
const char*  deviceNode  = "/dev/bf329x";
#endif 

int32_t btl_hw_openDevice()
{
    return open(deviceNode,O_RDWR);
}

int32_t btl_hw_setInterruptMode(int32_t device_fd)
{
    int32_t rx_ret = 0;
    rx_ret = ioctl(device_fd,BL229X_INTERRUPT_MODE);//进入中断模式
    return 0;
}



int32_t btl_hw_setPowerDwMode(int32_t device_fd)
{
    int32_t rx_ret = 0;
    rx_ret = ioctl(device_fd,BL229X_POWERDOWN_MODE);
    return 0;
}

int32_t btl_hw_get_chip_info(int32_t device_fd, struct bl229x_chip_info *chip_info)
{
	int32_t rx_ret =0;
	uint32_t drv_type;

	rx_ret = ioctl(device_fd,BL229X_GET_CHIP_INFO,&drv_type);
	if (rx_ret !=0 ){
		LOGD("BL229X_GET_CHIP_INFO failed! ");
		return -1;
	}
	chip_info->chip_driver_type = drv_type;
		LOGD("BL229X_GET_CHIP_INFO drv_type=%d ",drv_type);
	return 0;
}

int32_t btl_hw_getParams(int32_t device_fd,struct fingerprintd_params_t* p_fingerprintd_params )
{
    int32_t rx_ret = -1;

    if (device_fd == -1 || device_fd == 0) return 0;
    rx_ret = ioctl(device_fd,BL229X_INIT_ARGS,p_fingerprintd_params);
    if (rx_ret == 0) {
		/*
        LOGD("%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
             __func__,
             p_fingerprintd_params->def_contrast,
             p_fingerprintd_params->def_ck_fingerup_contrast,
             p_fingerprintd_params->reserved0,
             p_fingerprintd_params->reserved1,
             p_fingerprintd_params->def_match_failed_times,
             p_fingerprintd_params->def_enroll_try_times,
             p_fingerprintd_params->def_match_try_times,
             p_fingerprintd_params->def_intensity_threshold,
             p_fingerprintd_params->def_contrast_high_value,
             p_fingerprintd_params->def_contrast_low_value,
             p_fingerprintd_params->def_match_quality_score_threshold,
             p_fingerprintd_params->def_match_quality_area_threshold,
             p_fingerprintd_params->def_enroll_quality_score_threshold,
             p_fingerprintd_params->def_enroll_quality_area_threshold,
             p_fingerprintd_params->def_shortkey_disable,
             p_fingerprintd_params->def_rate,
             p_fingerprintd_params->def_max_samples,
             p_fingerprintd_params->def_debug_enable,
             p_fingerprintd_params->def_contrast_direction);
             */
        LOGD("get params from hw");
    } else {
        LOGE("%s,init err",__func__);
    }
    return rx_ret;
}

