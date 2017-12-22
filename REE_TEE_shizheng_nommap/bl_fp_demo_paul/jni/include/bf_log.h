#ifndef __BF_LOG_H__
#define __BF_LOG_H__
#include "bf_custom.h"
#define LOG_TAG "paultest"
#ifndef BUILD_TEE
#include <android/log.h>

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG,__VA_ARGS__)

#define BF_LOG(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG,"%5d: <%s>" fmt "\n",  __LINE__, __func__, ##args)
#define CF_ENTRY() 	LOGD("+++")
#define CF_EXIT() 	LOGD("---")
#define BTL_DEBUG(fmt, args...) __android_log_print(ANDROID_LOG_INFO, "blestech","%5d: <%s>" fmt "\n",  __LINE__, __func__, ##args)

#else//BUILD_TEE

#if defined (TK_TEE)
#include <trace.h>

#define BF_LOG(...) 	trace_printf_helper(TRACE_ERROR, true, __VA_ARGS__)
#define BTL_DEBUG(...)  trace_printf_helper(TRACE_ERROR, true, __VA_ARGS__)
#define BF_LOG_INFO(...)   trace_printf_helper(TRACE_ERROR, true, __VA_ARGS__)
#define BF_LOG_DEBUG(...)  trace_printf_helper(TRACE_ERROR, true, __VA_ARGS__)
#define BF_LOG_FLOW(...)   trace_printf_helper(TRACE_ERROR, true, __VA_ARGS__)
#endif

#if defined (ISEE_TEE)
#include <l4/log/log.h>
#define BF_LOG(...)   LOG_DBG("[btl_ta] " __VA_ARGS__)
#define BTL_DEBUG LOG_DBG
#endif

#if defined (ISEE25_TEE)
#include <ut_sys_util.h>
#define BF_LOG(...)   ut_sys_log("[btl_ta] " __VA_ARGS__)
#define BTL_DEBUG(...)   ut_sys_log("[btl_ta] " __VA_ARGS__)
#endif


#if defined (RSEE_TEE)
#include <trace.h>
#define BF_LOG(fmt, args...) do { \
        EMSG("[btl_ta]: +%d[%s] "fmt, __LINE__, __func__, ##args);  \
    } while (0)

#define BTL_DEBUG(fmt, args...) do { \
        EMSG("[btl_ta]: +%d[%s] "fmt, __LINE__, __func__, ##args);  \
    } while (0)
#endif


#if defined (TRUSTONIC_TEE)
#include <taStd.h>
#include <tee_internal_api.h>

#define BF_LOG(fmt, ...) do { \
        TEE_DbgPrintLnf("[bf_ta]: +%d[%s] "fmt, __LINE__, __func__, ##__VA_ARGS__);  \
    } while (0)

#define BTL_DEBUG(fmt, ...) do { \
        TEE_DbgPrintLnf("[bf_ta]: +%d[%s] "fmt, __LINE__, __func__, ##__VA_ARGS__);  \
    } while (0)
#endif



#if defined (TRUSTY_TEE)

#include <trusty_std.h>
#include <stdio.h>
#define LOG_TAG "[bf_ta]"
#define   BF_LOG(fmt, ...)        fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)
#define   BTL_DEBUG(fmt, ...)     fprintf(stderr, "%s: %d: " fmt, LOG_TAG, __LINE__,  ## __VA_ARGS__)

#endif



//LOG_ERR
#endif


#endif
