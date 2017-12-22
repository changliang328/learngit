/*
 * Copyright (C) 2015 Fingerprint Cards AB
 * All rights reserved.
 */

#ifndef INCLUSION_GUARD_FP_TA_DEBUG
#define INCLUSION_GUARD_FP_TA_DEBUG

#ifndef __BLACKBOX_TEST__
//#include <qsee_log.h>
//TODO add log level control here , where qc control here?
#include <l4/log/log.h>
#endif

#ifdef  ENABLE_QSEE_LOG_MSG_LOW
#undef  ENABLE_QSEE_LOG_MSG_LOW
#ifdef  FP_TA_INFO_LOG
#define ENABLE_QSEE_LOG_MSG_LOW    1
#else
#define ENABLE_QSEE_LOG_MSG_LOW    0
#endif
#endif

#ifdef  ENABLE_QSEE_LOG_MSG_ERROR
#undef  ENABLE_QSEE_LOG_MSG_ERROR
#ifdef  FP_TA_ERROR_LOG
#define ENABLE_QSEE_LOG_MSG_ERROR  1
#else
#define ENABLE_QSEE_LOG_MSG_ERROR  0
#endif
#endif

#ifdef  ENABLE_QSEE_LOG_MSG_MED
#undef  ENABLE_QSEE_LOG_MSG_MED
#ifdef  FP_TA_INFO_LOG
#define ENABLE_QSEE_LOG_MSG_MED    1
#else
#define ENABLE_QSEE_LOG_MSG_MED    0
#endif
#endif

#ifdef  ENABLE_QSEE_LOG_MSG_HIGH
#undef  ENABLE_QSEE_LOG_MSG_HIGH
#ifdef  FP_TA_INFO_LOG
#define ENABLE_QSEE_LOG_MSG_HIGH   1
#else
#define ENABLE_QSEE_LOG_MSG_HIGH   0
#endif
#endif

#ifdef  ENABLE_QSEE_LOG_MSG_FATAL
#undef  ENABLE_QSEE_LOG_MSG_FATAL
#define ENABLE_QSEE_LOG_MSG_FATAL  0
#endif

#ifdef  ENABLE_QSEE_LOG_MSG_DEBUG
#undef  ENABLE_QSEE_LOG_MSG_DEBUG
#ifdef  FP_TA_DEBUG_LOG
#define ENABLE_QSEE_LOG_MSG_DEBUG  1
#else
#define ENABLE_QSEE_LOG_MSG_DEBUG  0
#endif
#endif

#endif // INCLUSION_GUARD_FP_TA_DEBUG
