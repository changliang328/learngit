/******************** (C) COPYRIGHT 2015 GLIMMER ********************************
* File Name          : log.h
* Author               : guoyaowei
* Version              : 1.0
* Date                  : 2017.2.21
*******************************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <android/log.h>
#define DEBUG 1
#if DEBUG
#define LOGD(TAG, ...) 		__android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(TAG, ...) 		__android_log_print(ANDROID_LOG_INFO, TAG,  __VA_ARGS__)
#define LOGW(TAG, ...)		__android_log_print(ANDROID_LOG_WARN, TAG,  __VA_ARGS__)
#define LOGE(TAG, ...) 		__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGF(TAG, ...) 		__android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__)
#else
#define LOGD(TAG, ...)
#define LOGI(TAG, ...)
#define LOGW(TAG, ...)
#define LOGE(TAG, ...)
#define LOGF(TAG, ...)
#endif
#endif	/* __LOG_H__ */
