#ifndef __BF_LOG_H__
#define __BF_LOG_H__

#define LOG_TAG "paultest"
#include <android/log.h>

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG,__VA_ARGS__)

#define BF_LOG(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG,"%5d: <%s>" fmt "\n",  __LINE__, __func__, ##args)

//#define BTL_DEBUG(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG,"%5d: <%s>" fmt "\n",  __LINE__, __func__, ##args)
#define BTL_DEBUG(fmt, args...) 

#endif
