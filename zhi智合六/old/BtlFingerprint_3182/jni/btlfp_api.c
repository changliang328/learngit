/*
 * Copyright (C) 2016 BetterLife.Co.Ltd. All rights  reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include <time.h>
#include <linux/wait.h>
#include <pthread.h>
#include <hardware_legacy/power.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include "sqlite3.h"
#include "btlfp.h"





const char *strDataDbName = "fingerprint.db";
const char *preStoredPath        = "/data/system/users/";
char        mWorkedPath[PATH_SIZE];
const char *wakeid  = "btlfinger";

extern const char* deviceNode;
const char* halLibVer = "v4.0";
const char* releaser = "manhongjie";

const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_BTL_BASE = FINGERPRINT_ACQUIRED_VENDOR_BASE + 100;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT = FINGERPRINT_ACQUIRED_VENDOR_BASE + 101;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_FINGER_DOWN = FINGERPRINT_ACQUIRED_VENDOR_BASE + 102;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_FINGER_UP = FINGERPRINT_ACQUIRED_VENDOR_BASE + 103;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = FINGERPRINT_ACQUIRED_VENDOR_BASE + 104;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = FINGERPRINT_ACQUIRED_VENDOR_BASE + 105;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_DUPLICATE_AREA = FINGERPRINT_ACQUIRED_VENDOR_BASE + 106;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_LOW_COVER = FINGERPRINT_ACQUIRED_VENDOR_BASE + 107;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_BAD_IMAGE = FINGERPRINT_ACQUIRED_VENDOR_BASE + 108;


extern uint16_t  g_match_ck_fingerup_timeout;
extern uint16_t  g_enroll_ck_fingerup_timeout;
extern uint8_t   g_match_failed_times;
extern uint8_t   g_shotkey_disable;
extern uint8_t   g_enroll_max_counts;
extern uint8_t   g_algorithm_type;
int g_fid_subindex = 0;
int g_fid_index = 0;

typedef struct fingerprint_item {
    hw_auth_token_t auth_token;
    uint32_t  fid;
    uint32_t  gid;
    uint32_t  used;
} fingerprint_item_t;


typedef struct fingerprint_param {
    fingerprint_device_t *dev;
    hw_auth_token_t auth_token;
    uint32_t gid;
    uint32_t fid;
    uint32_t timeout_sec;
    uint64_t operation_id;
} fingerprint_param_t;


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

fingerprint_param_t  gFingerprintParam;
fingerprint_var_t    gFingerprintVar;

static void btl_api_createWorkedPath(char *workPath, uint32_t gid)
{
    char  tempBuf[10];
    const char *fpdata = "fpdata/";
    LOGD("++ %s", __func__);
    strcpy(workPath, preStoredPath);
    sprintf(tempBuf, "%d/", gid);
    strcat(workPath, tempBuf);
    strcat(workPath, fpdata);
    LOGD("%s", workPath);
    LOGD("-- %s", __func__);
}

static void btl_api_getDbFilename(char *dbFilename, uint32_t gid)
{
    char  tempBuf[10];
    const char *fpdata = "fpdata/";
    LOGD("++ %s", __func__);
    strcpy(dbFilename, preStoredPath);
    sprintf(tempBuf, "%d/", gid);
    strcat(dbFilename, tempBuf);
    strcat(dbFilename, fpdata);
    strcat(dbFilename, strDataDbName);
    //LOGD("%s", dbFilename);
    LOGD("-- %s", __func__);
}
static int32_t btl_api_getValidFingerId()
{
    int32_t i, j, size;
    int32_t fid_array[10];
    char    dbName[PATH_SIZE];
    int     count = 0;
    const   int id[MAX_FINGER_NUMS] = {1, 2, 3, 4, 5};
    int     equal = 0;
    int     fid = 0;
    LOGD("++ %s", __func__);
    btl_api_getDbFilename(dbName, gFingerprintVar.gid);
    count = sql_inquire_fingerpint_fid(dbName, fid_array);

    if(count == 0) return fid;

    if(count == 5) return MAX_FINGER_NUMS;

    for(i = 0; i < count; i++) {
        LOGD("fid:%d \n", fid_array[i]);
    }

    for(i = 0; i < MAX_FINGER_NUMS; i++) {
        for(j = 0, equal = 0; j < count; j++) {
            if(id[i] == fid_array[j]) {
                equal = 1;
                break;
            }
        }

        if(equal == 0) {
            fid = id[i];
            break;
        }
    }

    LOGD("new fid: %d", fid);
    LOGD("-- %s", __func__);
    return fid;
}

/*----------------------------------------------------------------------
purpose:   read finger index from data file
return :   int32_t
------------------------------------------------------------------------*/
static int32_t btl_api_createDB(char *dbName)
{
    sql_create_database(dbName);
    return 0;
}

/*----------------------------------------------------------------------
purpose: Generate a filename by finger id
return :   void
------------------------------------------------------------------------*/
static void btl_api_generateFileName(char *strFileName, int32_t fid, int32_t gid)
{
    char tmpbuf[10];
    btl_api_createWorkedPath(strFileName, gid);
    sprintf(tmpbuf, "%d", fid);
    strcat(strFileName, tmpbuf);
}

static int32_t btl_api_getFingerprintData(uint8_t *pTemplateBuf, uint32_t *pSize)
{
    char tempBuf[128];
    LOGD("++ %s", __func__);
    LOGD("%s:%d,%d", __func__, gFingerprintVar.fid, gFingerprintVar.gid);
    btl_core_getTemplate(pTemplateBuf, pSize);
    LOGD("-- %s", __func__);
    return 0;
}

static void btl_api_setInterruptMode()
{
    btl_core_setInterruptWorkMode();
}

static void btl_api_setCancel(uint8_t flag)
{
    if(flag) {
        gFingerprintVar.cancel = 1;
        btl_core_cancelSensor(1);
    } else {
        gFingerprintVar.cancel = 0;
        btl_core_cancelSensor(0);
    }
}

static uint64_t btl_api_createToken()
{
    uint64_t token = 0;
    struct timespec realtime = {0, 0};
    LOGD("%s", __func__);
    clock_gettime(CLOCK_MONOTONIC, &realtime);
    token = realtime.tv_nsec;
    return token;
}

static int32_t btl_api_initEnroll()
{
    btl_core_enrollStart();

    return 0;
}

int32_t btl_api_registeFp(int32_t *pRadio)
{
    return btl_core_registeFp(pRadio);
}

static void btl_api_responseFunc(int __unused signum)
{
    LOGD("%s", __func__);
    pthread_cond_signal(&gFingerprintVar.count_nonzero);
}

static void btl_api_setAgc(uint32_t contrast)
{
    LOGD("%s", __func__);
    btl_core_setAgc(contrast);
}

static int32_t btl_api_waitSignal(int32_t timeout_sec,int32_t timeout_ms)
{
    int ret;
    int Oflags;
    int counts = 0;
    int retWait = 0;
    LOGD("++ %s", __func__);


    if(gFingerprintVar.cancel == 1)
        return -3;

    pthread_mutex_lock(&gFingerprintVar.count_lock);
    ret = 0;
    signal(SIGIO, &btl_api_responseFunc);
    int fd = open(deviceNode, O_RDWR);
    if(fd < 0) {
        LOGD("can't open device");
        pthread_mutex_unlock(&gFingerprintVar.count_lock);
        return -1 ;
    }
    fcntl(fd, F_SETOWN, getpid());
    Oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, Oflags | FASYNC);


    if(gFingerprintVar.cancel != 1) {
        if(timeout_sec == 0xff)
            pthread_cond_wait(&gFingerprintVar.count_nonzero,
                              &gFingerprintVar.count_lock);
        else {
            struct timespec timer;
            struct timeval now;
            memset(&timer, 0, sizeof(struct timespec));
            memset(&now, 0, sizeof(struct timeval));
            gettimeofday(&now, NULL);
            timer.tv_sec = now.tv_sec + timeout_sec;
            timer.tv_nsec = now.tv_usec * 1000+timeout_ms*1000*1000;
            retWait = pthread_cond_timedwait(&gFingerprintVar.count_nonzero,
                                             &gFingerprintVar.count_lock,
                                             &timer);
            if(retWait == ETIMEDOUT) {
                LOGD("timeout");
                ret = -2;
            }
        }
    }

    if(gFingerprintVar.cancel == 1)
        ret = -3;

    close(fd);
    pthread_mutex_unlock(&gFingerprintVar.count_lock);
    LOGD("-- %s", __func__);
    return ret;
}


static void btl_api_isFingerUp(int32_t check_timeout_ms)
{
    uint8_t ck_fingerup = 1;
    int32_t retWait = 0;

    btl_api_setInterruptMode();
    while (gFingerprintVar.cancel != 1 && ck_fingerup == 1) {
        retWait = btl_api_waitSignal(0,check_timeout_ms);
        if (retWait == -2)
            ck_fingerup = 0;
    }
    return;
}

static void *thEnrollFunc(void *arg)
{
    fingerprint_device_t *dev;
    fingerprint_param_t  *param;
    fingerprint_msg_t msg;
    int32_t enrollStatus = 0;
    char    dbName[PATH_SIZE];
    int32_t validFid = 0;
    int32_t upCheckCounts = 0;
    int32_t timeout_sec = 0;
    int32_t errorCounts = 0;
    int32_t retWait = 0;
    uint8_t *pTemplateBuf = NULL;
    uint32_t template_size = 0;
    uint8_t  fingerup_counts = 0;
    int32_t  upRet = 0;
    int32_t  radio = 0;
    int32_t  lastRadio = 0;
    uint8_t  roundloop = 0;
    int32_t index[10];
    int32_t index_length = 0;
    int     fingeId = 0;

	
    LOGD("++ %s", __func__);
    // create worked directory by gid, created method is correct ???

	pthread_detach(pthread_self());
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    pTemplateBuf = malloc(100 * 1024);

    if(pTemplateBuf == NULL) {
        pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
        LOGE("%s, can't allocate buffer", __func__);
        return ((void *)0);
    }

    param = (fingerprint_param_t *)arg;
    dev = param->dev;
    gFingerprintVar.gid = param->gid;
    timeout_sec =  param->timeout_sec;
    gFingerprintVar.cancel  = 0;
    memset(dbName, 0, PATH_SIZE);
    btl_api_getDbFilename(dbName, gFingerprintVar.gid);
    gFingerprintVar.fid = btl_api_getValidFingerId();

    if(gFingerprintVar.fid == 0) gFingerprintVar.fid = 1;
    #if FINGER_DATA_FILE
    fingeId = btl_file_fingerid();
    gFingerprintVar.fid = fingeId;
    LOGD("fingeId = %d\n", fingeId);
    #endif
	
    btl_api_initEnroll();

    index_length = sql_inquire_fingerpint_fid(dbName, index);
    g_fid_subindex = 0;
    do {   
        retWait = 0;
        btl_api_setInterruptMode();
        retWait = btl_api_waitSignal(0xff,0);
        if(retWait == -3) break;
        if(retWait == -2) {
            memset(&msg, 0, sizeof(fingerprint_msg_t));
            msg.type = FINGERPRINT_ERROR;
            msg.data.error = FINGERPRINT_ERROR_TIMEOUT;
            dev->notify(&msg);
            LOGE("Fp_Enroll timeout");
            break;
        }
		memset(&msg, 0, sizeof(fingerprint_msg_t));
        msg.type = FINGERPRINT_ACQUIRED;
        msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
        dev->notify(&msg);
		g_fid_index = index_length+1;
        enrollStatus = btl_api_registeFp(&radio);
        LOGD("%s:%d", __func__, enrollStatus);

        if(enrollStatus == 0) {

            //clear previous's counts of error
            errorCounts = 0;

            if(radio == g_enroll_max_counts) {
                btl_api_getFingerprintData(pTemplateBuf, &template_size);
				#if FINGER_DATA_FILE
				btl_file_saveTemplate(pTemplateBuf, template_size, fingeId);
				#else
                sql_insert_fingerprint(dbName, gFingerprintVar.gid, gFingerprintVar.fid, pTemplateBuf, template_size);
				#endif
                btl_api_setCancel(1);
            }

            if (radio != lastRadio) {
                g_fid_subindex ++;
                memset(&msg, 0, sizeof(fingerprint_msg_t));
                msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
                msg.data.enroll.finger.fid = gFingerprintVar.fid;
                msg.data.enroll.finger.gid = gFingerprintVar.gid;
                msg.data.enroll.samples_remaining = radio > g_enroll_max_counts ? 0: g_enroll_max_counts - radio;
                LOGD("Fp_Enroll samples_remaining :%d", msg.data.enroll.samples_remaining);
                dev->notify(&msg);
                //record current radio
                lastRadio = radio;
                //check if enrollment is completed
                if(radio == g_enroll_max_counts)break;
            } else {
                memset(&msg, 0, sizeof(fingerprint_msg_t));
                msg.type = FINGERPRINT_ACQUIRED;
                msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_DUPLICATE_FINGER;
                LOGD("FINGERPRINT_ACQUIRED_DUPLICATE_FINGER samples_remaining :%d", msg.data.enroll.samples_remaining);
                dev->notify(&msg);
                LOGD("Fp_Enroll radio = lastradio :%d", lastRadio);
                continue;
            }
            //check if finger up
            btl_api_isFingerUp(g_enroll_ck_fingerup_timeout);
        } else if(enrollStatus ==-ERR_AGC || enrollStatus == -ERR_ENROLL) {
            msg.type = FINGERPRINT_ACQUIRED;
            msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_PARTIAL;
            dev->notify(&msg);
            errorCounts = 0;
        } else if(enrollStatus == -ERR_EMPTY) {
            errorCounts = 0;
            continue;
        } else if(enrollStatus == -ERR_CANCEL) {
            break;
        } else
            errorCounts++;

        if(errorCounts > DEFAULT_MAX_ERROR_COUNTS && gFingerprintVar.cancel != 1) {
            memset(&msg, 0, sizeof(fingerprint_msg_t));
            msg.type = FINGERPRINT_ERROR;
            msg.data.error = FINGERPRINT_ERROR_TIMEOUT;
            dev->notify(&msg);
            LOGE("Fp_Enroll can't process");
            break;
        }
    } while(radio < g_enroll_max_counts && gFingerprintVar.cancel != 1);

    if (btl_core_getMteStatus())
        btl_core_unInitAlgo();
    btl_api_setInterruptMode();
	free((void *)pTemplateBuf);
    pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
    LOGD("-- %s", __func__);
    return ((void *)0);
}


void *thAuthenticateFunc(void *arg)
{
    int32_t templatelength = 0;
    int32_t index[10];
    int32_t index_length = 0;
    int32_t authenticateStatus = 0;
    int32_t fingerID = 0;
    int32_t ret_value = 0;
    int32_t errorCounts = 0;
    int32_t matchErrors = 0;
    char    dbName[PATH_SIZE];
    fingerprint_msg_t msg;
    fingerprint_device_t *dev;
    fingerprint_param_t  *param;
    int32_t retWait = 0;

	pthread_detach(pthread_self());
    LOGD("++ %s", __func__);
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    param = (fingerprint_param_t *)arg;
    dev = param->dev;
    gFingerprintVar.gid  = param->gid;
    gFingerprintVar.cancel = 0;
    memset(&msg, 0, sizeof(fingerprint_msg_t));
    btl_api_getDbFilename(dbName, gFingerprintVar.gid);
	#if FINGER_DATA_FILE
	index_length = btl_file_fingerNum(index);
	LOGD("%s len:%d", __func__, index_length);
	int i;
	for(i=0;i<index_length; i++)
		LOGD("index[%d] = %d", i, index[i]);
	#else
    index_length = sql_inquire_fingerpint_fid(dbName, index);
    LOGD("%s, gid:%d, len:%d", __func__, gFingerprintVar.gid, index_length);
    #endif
    if(index_length == 0) {
        memset(&msg, 0, sizeof(fingerprint_msg_t));
    }

    while(gFingerprintVar.cancel != 1 && index_length != 0) {
        btl_api_setInterruptMode();
        btl_api_waitSignal(0xff,0);
        if (gFingerprintVar.cancel)break;
        LOGD("Finger is pressed");
        memset(&msg, 0, sizeof(fingerprint_msg_t));
        msg.type = FINGERPRINT_ACQUIRED;
        msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
        dev->notify(&msg);

        fingerID = 0;
        authenticateStatus = btl_core_matchFpC(dbName, index, index_length, (int32_t)gFingerprintVar.gid,&fingerID);
        LOGD("thAuthenticateFunc match fingerid:%d, %d", fingerID, authenticateStatus);

        if(authenticateStatus == -ERR_CANCEL) {  //match cancel
            LOGD("%s,authenticate cancel", __func__);
            break;
        }

        memset(&msg, 0, sizeof(fingerprint_msg_t));
        msg.type = FINGERPRINT_ACQUIRED;
        msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
        dev->notify(&msg);

        if(authenticateStatus == DECISION_MATCH) {
            memset(&msg, 0, sizeof(fingerprint_msg_t));
            msg.type = FINGERPRINT_AUTHENTICATED;
            msg.data.authenticated.finger.gid = gFingerprintVar.gid ;
            msg.data.authenticated.finger.fid = fingerID;
            msg.data.authenticated.hat.user_id = gFingerprintVar.gid;
            msg.data.authenticated.hat.authenticator_id = gFingerprintVar.token;
            uint32_t tmp = (uint32_t)HW_AUTH_FINGERPRINT;
            msg.data.authenticated.hat.authenticator_type = btl_util_my_hton(tmp);
            msg.data.authenticated.hat.challenge = gFingerprintParam.operation_id;
            msg.data.authenticated.hat.timestamp = btl_util_my_htonl(btl_api_createToken());
            dev->notify(&msg);
            LOGD("Fp_Authenticate fingerid:%d", msg.data.authenticated.finger.fid);
            break;
        } else if(authenticateStatus == -ERR_EMPTY || authenticateStatus == -ERR_AGC ) {
            if (authenticateStatus == -ERR_AGC) {
                memset(&msg, 0, sizeof(fingerprint_msg_t));
                msg.type = FINGERPRINT_ACQUIRED;
                msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
                dev->notify(&msg);
            }
            if (gFingerprintVar.cancel)break;
            continue;
        } else if(authenticateStatus == DECISION_NON_MATCH) { //match fail
            memset(&msg, 0, sizeof(fingerprint_msg_t));
            msg.type = FINGERPRINT_AUTHENTICATED;
            msg.data.authenticated.finger.gid = gFingerprintVar.gid;
            msg.data.authenticated.finger.fid  = 0;
            dev->notify(&msg);
            matchErrors++;

            if (matchErrors >= g_match_failed_times)   {
                LOGE("matchErrors > %d",matchErrors);
                break;
            }
            btl_api_isFingerUp(g_match_ck_fingerup_timeout);
        }
    }
    if(g_shotkey_disable != 1 && gFingerprintVar.cancel != 1) {
        LOGD("%s,check fp up for shotkey",__func__);
        btl_api_isFingerUp(g_match_ck_fingerup_timeout);
    }
	btl_core_initIC();
    btl_api_setInterruptMode();
    pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
    LOGD("-- %s", __func__);
    return ((void *)0);
}

int32_t Fp_InitLock()
{
    pthread_mutex_init(&gFingerprintVar.count_lock, NULL);
    pthread_cond_init(&gFingerprintVar.count_nonzero, NULL);
    pthread_mutex_init(&gFingerprintVar.fingerprint_lock, NULL);
    gFingerprintVar.fid = 0;
    gFingerprintVar.gid = 0;
    gFingerprintVar.cancel = 0;
    btl_api_setInterruptMode();
    btl_core_getDefaultParams();
	LOGI("%s",halLibVer);
	LOGI("release by %s",releaser);
    return 0;
}


int32_t Fp_PostEnroll(fingerprint_device_t __unused *dev)
{
    LOGD("%s", __func__);
    return 0;
}


int32_t Fp_Cancel(fingerprint_device_t __unused *dev)
{
    fingerprint_msg_t msg;

    LOGD("++ %s", __func__);
    btl_api_setCancel(1);
    gFingerprintVar.cancel = 1;
    pthread_cond_signal(&gFingerprintVar.count_nonzero);
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    btl_api_setInterruptMode();
    pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
#ifdef BTL_ANDROID_N
    memset(&msg, 0, sizeof(fingerprint_msg_t));
    msg.type = FINGERPRINT_ERROR;
    msg.data.error = 5 ;
    dev->notify(&msg);
    //LOGD("Fp_Cancel fingerid:%d", msg.data.authenticated.finger.fid);
#endif

    LOGD("-- %s", __func__);
    return 0;
}


int32_t Fp_Enroll(fingerprint_device_t __unused *dev, const hw_auth_token_t __unused * token,
                  uint32_t __unused gid, uint32_t __unused timeout_sec)
{
    int32_t enrollStatus = 0;
    int err;
    LOGD("++ %s", __func__);
    btl_api_setCancel(1);
    pthread_cond_signal(&gFingerprintVar.count_nonzero);
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
    btl_api_setCancel(0);
    memset(&gFingerprintParam, 0, sizeof(fingerprint_param_t));
    memcpy(&gFingerprintParam.auth_token, token, sizeof(hw_auth_token_t));
    gFingerprintParam.dev = dev;
    gFingerprintParam.gid = gid;
    gFingerprintParam.timeout_sec = timeout_sec;
    LOGD("%s, timeout:%d", __func__, timeout_sec);
    err = pthread_create(&gFingerprintVar.tid, NULL, &thEnrollFunc, &gFingerprintParam);

    if(err != 0) {
        LOGE("create thread error: %s/n", strerror(err));
        return -1;
    }

    LOGD("-- %s", __func__);
    return 0;
}


int32_t Fp_Authenticate(fingerprint_device_t __unused *dev, uint64_t __unused operation_id,
                        __unused uint32_t gid)
{
    int err;
    LOGD("++ %s", __func__);
    btl_api_setCancel(1);
    pthread_cond_signal(&gFingerprintVar.count_nonzero);
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
    memset(&gFingerprintParam, 0, sizeof(fingerprint_param_t));
    btl_api_setCancel(0);
    gFingerprintParam.dev = dev;
    gFingerprintParam.gid = gid;
    gFingerprintParam.operation_id = operation_id;
    err = pthread_create(&gFingerprintVar.tid, NULL, &thAuthenticateFunc, &gFingerprintParam);

    if(err != 0) {
        LOGE("create thread error: %s/n", strerror(err));
        return -1;
    }

    LOGD("-- %s", __func__);
    return 0;
}


int32_t Fp_SetActiveGroup(fingerprint_device_t __unused *dev, uint32_t __unused gid,
                          const char __unused *store_path)
{
    char dbName[PATH_SIZE];
    LOGD("++ %s", __func__);
    gFingerprintVar.gid  = gid;
    strcpy(mWorkedPath, store_path);
    LOGD("%s,%s", __func__, store_path);
    btl_api_getDbFilename(dbName, gFingerprintVar.gid);
    LOGD("%s,%s", __func__, dbName);
    btl_api_createDB(dbName);
	btl_api_createFile();
    LOGD("-- %s", __func__);
    return 0;
}



uint64_t Fp_PreEnroll(struct fingerprint_device __unused *dev)
{
    LOGD("++ %s", __func__);
    char dbName[PATH_SIZE];
    btl_api_getDbFilename(dbName, gFingerprintVar.gid);

    if(sql_inquire_all_fingerprint(dbName) == MAX_FINGER_NUMS) return 0;

    btl_api_setCancel(0);
    gFingerprintVar.token = btl_api_createToken();
    LOGD("-- %s", __func__);
    return gFingerprintVar.token;
}


int32_t Fp_Remove(fingerprint_device_t __unused *dev, uint32_t __unused gid,
                  uint32_t __unused fid)
{
    char dbFilename[PATH_SIZE];
    int32_t i ;
    int32_t fids_index[10];
    int32_t fids_size = 0;
    fingerprint_msg_t msg;


    LOGD("++ %s", __func__);
    btl_api_setCancel(1);
    pthread_cond_signal(&gFingerprintVar.count_nonzero);
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    LOGD("Fp_Remove:%d,%d", fid, gid);
    btl_api_getDbFilename(dbFilename, gid);
    if (fid > 0) {
		#if FINGER_DATA_FILE
		file_delete_fingerprint(fid);
		#else
        sql_delete_fingerprint(dbFilename, gid, fid);    
		#endif
        memset(&msg, 0, sizeof(fingerprint_msg_t));
        msg.type = FINGERPRINT_TEMPLATE_REMOVED;
        msg.data.removed.finger.gid = gid;
        msg.data.removed.finger.fid = fid;
        dev->notify(&msg);
    } else if (fid == 0) {
        LOGD("Fp_Remove:%d,%d", fid, gid);
        fids_size = sql_inquire_fingerpint_fid(dbFilename, fids_index);
        LOGD("%s, gid:%d, len:%d", __func__, gFingerprintVar.gid, fids_size);
        for(i = 0; i < fids_size; i++) {
            fid = fids_index[i];
            LOGD("%s, fid:%d", __func__, fid);
            sql_delete_fingerprint(dbFilename, gid, fid);
            memset(&msg, 0, sizeof(fingerprint_msg_t));
            msg.type = FINGERPRINT_TEMPLATE_REMOVED;
            msg.data.removed.finger.gid = gid;
            msg.data.removed.finger.fid = fid;
            dev->notify(&msg);
        }
    }
#ifdef BTL_ANDROID_N
    memset(&msg, 0, sizeof(fingerprint_msg_t));
    msg.type = FINGERPRINT_ERROR;
    msg.data.error = 5 ;
    dev->notify(&msg);
#endif
    btl_api_setCancel(0);
    pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
    LOGD("-- %s", __func__);
    return 0;
}

#ifdef BTL_ANDROID_N
int32_t Fp_Enumerate(fingerprint_device_t *dev)
{
    int32_t i = 0;
    char dbFilename[PATH_SIZE];
    int32_t fid[10];
    int32_t count;
    fingerprint_msg_t msg;


    LOGD("++ %s", __func__);
    btl_api_getDbFilename(dbFilename, gFingerprintVar.gid);
    count = sql_inquire_fingerpint_fid(dbFilename, fid);

    for(i = 0; i < count; i++) {
        msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
        msg.data.enumerated.finger.gid = gFingerprintVar.gid;
        msg.data.enumerated.finger.fid = fid[i];
        msg.data.enumerated.remaining_templates = count - i - 1;
        dev->notify(&msg);
    }
    LOGD("-- %s", __func__);
    return 0;
}
int64_t Fp_GetAuthId(fingerprint_device_t __unused *dev)
{
    LOGD("%s", __func__);
    return gFingerprintVar.token;
}
#else
int32_t Fp_Enumerate(fingerprint_device_t __unused *dev, fingerprint_finger_id_t __unused *results,
                     uint32_t *max_size)
{
    const char *id = "btlfingerprint";
    int32_t i, s;
    char dbFilename[PATH_SIZE];
    int32_t fid[10];
    int32_t  count;
    LOGD("++ %s", __func__);
    btl_api_getDbFilename(dbFilename, gFingerprintVar.gid);
    count = sql_inquire_fingerpint_fid(dbFilename, fid);

    for(i = 0; i < count; i++) {
        results[i].fid = fid[i];
        results[i].gid = 0;
    }

    *max_size = count;
    LOGD("-- %s", __func__);
    return 0;
}
int64_t Fp_GetAuthId(fingerprint_device_t __unused *dev)
{
    LOGD("%s", __func__);
    return gFingerprintVar.token;
}
#endif
