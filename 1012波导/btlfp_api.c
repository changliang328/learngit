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
#include <time.h>
#include <linux/wait.h>
#include <pthread.h>
#include <hardware_legacy/power.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include "sqlite3.h"
#include "btlfp.h"
#include "btl_algorithm_interface.h"



static int is_first_enter = 1;

const char *strDataDbName = "fingerprint.db";
const char *strDataBackupDbName = "fingerprint_backup.db";
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
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_ERR_ENROLL = FINGERPRINT_ACQUIRED_VENDOR_BASE + 6;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = FINGERPRINT_ACQUIRED_VENDOR_BASE + 104;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = FINGERPRINT_ACQUIRED_VENDOR_BASE + 5;//105;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_DUPLICATE_AREA = FINGERPRINT_ACQUIRED_VENDOR_BASE + 106;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_LOW_COVER = FINGERPRINT_ACQUIRED_VENDOR_BASE + 107;
const fingerprint_acquired_info_t    FINGERPRINT_ACQUIRED_BAD_IMAGE = FINGERPRINT_ACQUIRED_VENDOR_BASE + 6;//108;

extern uint16_t  g_match_ck_fingerup_timeout;
extern uint16_t  g_enroll_ck_fingerup_timeout;
extern uint8_t   g_match_failed_times;
extern uint8_t   g_shotkey_disable;
extern uint8_t   g_enroll_max_counts;
extern uint8_t   g_algorithm_type;
extern uint8_t g_width;
extern uint8_t g_height;
extern uint8_t g_update_far_value;
extern int UPDATE_AREA;

int g_fid_subindex = 0;
int g_fid_index = 0;
int g_en_responsefunc = 0;

extern uint8_t   g_online_update;
fingerprint_param_t  gFingerprintParam;
fingerprint_var_t    gFingerprintVar;
int    g_match_full;


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

static void btl_api_getDbFilename(char *dbFilename, uint32_t gid, int org)
{
    char  tempBuf[10];
    const char *fpdata = "fpdata/";
    LOGD("++ %s", __func__);
    strcpy(dbFilename, preStoredPath);
    sprintf(tempBuf, "%d/", gid);
    strcat(dbFilename, tempBuf);
    strcat(dbFilename, fpdata);
	if(org){
		strcat(dbFilename, strDataDbName);
	}else{
		strcat(dbFilename, strDataBackupDbName);
	}
    
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
    btl_api_getDbFilename(dbName, gFingerprintVar.gid, 1);
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
    LOGD("%s+++%d", __func__,g_en_responsefunc);
	if(g_en_responsefunc!=0){
		pthread_cond_signal(&gFingerprintVar.count_nonzero);
	}
	LOGD("%s---", __func__);
}

static void btl_api_setAgc(uint32_t contrast)
{
    LOGD("%s", __func__);
    btl_core_setAgc(contrast);
}

static void *thScheduleSetInterruptModeFunc(void *arg)
{
	LOGD("%s+++", __func__);
	pthread_detach(pthread_self());
    pthread_mutex_lock(&gFingerprintVar.count_lock);
	g_en_responsefunc = 1;
	btl_api_setInterruptMode();
	pthread_mutex_unlock(&gFingerprintVar.count_lock);
    return ((void *)0);
}

static int32_t btl_api_waitSignal(int32_t timeout_sec,int32_t timeout_ms)
{
    int ret = 0;
    int Oflags;
    int counts = 0;
    int retWait = 0;
	int err = 0;
    LOGD("++ %s", __func__);

	pthread_mutex_lock(&gFingerprintVar.count_lock);
    if(gFingerprintVar.cancel == 1){
		LOGD("btl_api_waitSignal cancel");
		pthread_mutex_unlock(&gFingerprintVar.count_lock);
		return -3;
	}
        

    int fd = open(deviceNode, O_RDWR);
    if(fd < 0) {
        LOGD("can't open device");
        pthread_mutex_unlock(&gFingerprintVar.count_lock);
        return -1 ;
    }

    signal(SIGIO, &btl_api_responseFunc);
    fcntl(fd, F_SETOWN, getpid());
    Oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, Oflags | FASYNC);

	err = pthread_create(&gFingerprintVar.tid, NULL, &thScheduleSetInterruptModeFunc, NULL);
    if(err != 0) {
        LOGE("create thread error: %s/n", strerror(err));
		pthread_mutex_unlock(&gFingerprintVar.count_lock);
        return -1;
    }

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
            timer.tv_sec = now.tv_sec+timeout_sec+timeout_ms/1000;
            timer.tv_nsec = now.tv_usec*1000+(timeout_ms%1000)*1000000;  
			if (timer.tv_nsec > 1000000000) {  
				timer.tv_sec += 1;  
				timer.tv_nsec -= 1000000000;  
			}
			
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
	g_en_responsefunc = 0;
    LOGD("-- %s", __func__);
    return ret;
}

extern uint16_t g_chip_id;
static int32_t btl_finger_up_mode(int32_t timeout_sec,int32_t timeout_ms)
{
	int ret = 0;
	pthread_mutex_lock(&gFingerprintVar.count_lock);
    int fd = open(deviceNode, O_RDWR);
    if(fd < 0) {
        LOGD("can't open device");
        pthread_mutex_unlock(&gFingerprintVar.count_lock);
        return -1 ;
    }
    LOGD("BL229X_WAIT_FINGER_UP++++");
    ret = ioctl(fd, BL229X_WAIT_FINGER_UP, 0);
    LOGD("BL229X_WAIT_FINGER_UP----ret=%d",ret);
    close(fd);
    pthread_mutex_unlock(&gFingerprintVar.count_lock);
    return ret;
}
static void btl_api_isFingerUp(int32_t check_timeout_ms)
{
    uint8_t ck_fingerup = 1;
    int32_t retWait = 0;

    //btl_api_setInterruptMode();
    if(g_chip_id == 0x5183)
    {
		while (gFingerprintVar.cancel != 1 && ck_fingerup == 1) {
		    retWait = btl_api_waitSignal(0,check_timeout_ms);
		    if (retWait == -2)
		        ck_fingerup = 0;
		}
    }
    else
    {
		while (ck_fingerup == 1) {
		    retWait = btl_finger_up_mode(0,check_timeout_ms);
		     LOGD("retWait=%d",retWait);
		     ck_fingerup = 0;
		}
    }
    return;
}


static void *thUpdateFunc(void *arg)
{
   unsigned char *fp_captured_image;
   unsigned char *update_template_data = 0;
   int update_template_size = 0;
   unsigned char update_template_type = 0;
   char    db_name[PATH_SIZE];
   int return_val = 0;
   int id;
   
   //pthread_detach(pthread_self());
   //pthread_mutex_lock(&gFingerprintVar.update_lock);
   if (gFingerprintVar.dyn_update_data.captured_image == 0){
   	  if(gFingerprintVar.dyn_update_data.matched_template_data)
	  	 free(gFingerprintVar.dyn_update_data.matched_template_data);
	  gFingerprintVar.dyn_update_data.matched_template_data = 0;
	  //pthread_mutex_unlock(&gFingerprintVar.update_lock);
      return ((void *)0);
   	}
   update_template_data = malloc(TEMPLATE_SIZE+1);
   if (update_template_data  == 0) {
   	  free(gFingerprintVar.dyn_update_data.captured_image);
	  gFingerprintVar.dyn_update_data.captured_image = 0;
	  if (gFingerprintVar.dyn_update_data.matched_template_data)
	  	 free(gFingerprintVar.dyn_update_data.matched_template_data);
	  gFingerprintVar.dyn_update_data.matched_template_data = 0;
	  //pthread_mutex_unlock(&gFingerprintVar.update_lock);
	  return ((void *)0);
   }
   id = gFingerprintVar.matched_index;
   fp_captured_image = gFingerprintVar.dyn_update_data.captured_image;
   return_val = Btl_DynamicUpdate(
          fp_captured_image, 
		  g_width,
		  g_height,
		  g_update_far_value,
		  0,
		  gFingerprintVar.dyn_update_data.matched_template_data,
		  gFingerprintVar.dyn_update_data.matched_template_size,
		  &update_template_data[1],
		  &update_template_size,
		  &update_template_type);

      //update fignerprint data
    if (return_val == 0) {
        int32_t fid = gFingerprintVar.dyn_update_data.fid;
		int32_t uid = gFingerprintVar.dyn_update_data.uid;
		btl_api_getDbFilename(db_name, gFingerprintVar.gid, 1);
        if (update_template_size > 0) {
            update_template_data[0] = update_template_type;
            LOGD("size:%d, uid:%d, fid:%d, type:%d",update_template_size,uid,fid,update_template_type);
            update_template_size++;
            sql_delete_fingerprint(db_name, uid, fid);
            sql_insert_fingerprint(db_name, uid, fid, update_template_data, update_template_size);
            gFingerprintVar.is_need_reload_templates = 1;
        }
    }
	free(gFingerprintVar.dyn_update_data.captured_image);
	gFingerprintVar.dyn_update_data.captured_image = 0;
	if (gFingerprintVar.dyn_update_data.matched_template_data)
	  free(gFingerprintVar.dyn_update_data.matched_template_data);
	gFingerprintVar.dyn_update_data.matched_template_data = 0;
	gFingerprintVar.matched_index = 0;
	free(update_template_data);
	//pthread_mutex_unlock(&gFingerprintVar.update_lock);
    return ((void *)0);
}

extern uint32_t read_image_and_cal_dacp();
static void *thEnrollFunc(void *arg)
{
    fingerprint_device_t *dev;
    fingerprint_param_t  *param;
    fingerprint_msg_t msg;
    int32_t enrollStatus = 0;
    char    dbName[PATH_SIZE];
	char    dbBackupName[PATH_SIZE];
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
    /*add for match*/
#if MATCH_BEFORE_ENROLL
	int32_t i = 0;
	int32_t uid = 0;
    uint8_t* pTemplateBuffer[5];
	int32_t  nTemplateSize[5];
#endif
	/*add for match*/
    LOGD("++ %s", __func__);
    // create worked directory by gid, created method is correct ???

	pthread_detach(pthread_self());
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    pTemplateBuf = malloc(TEMPLATE_SIZE);

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
    btl_api_getDbFilename(dbName, gFingerprintVar.gid, 1);
    gFingerprintVar.fid = btl_api_getValidFingerId();

    if(gFingerprintVar.fid == 0) gFingerprintVar.fid = 1;
    #if FINGER_DATA_FILE
    fingeId = btl_file_fingerid();
    gFingerprintVar.fid = fingeId;
    LOGD("fingeId = %d\n", fingeId);
    #endif
	
    btl_api_initEnroll();

    index_length = sql_inquire_fingerpint_fid(dbName, index);
    
    /*add for match*/
#if MATCH_BEFORE_ENROLL
	uid = gFingerprintVar.gid;
	if(is_first_enter == 1)
	{
		is_first_enter = 0;
		for (i = 0; i < 5; i++){
			gFingerprintVar.enrolled_template[i].fp_template_data = malloc(TEMPLATE_SIZE);
			gFingerprintVar.enrolled_template[i].fp_template_size = 0;
			gFingerprintVar.enrolled_template[i].fp_template_type = 0;
		}
	}
    btl_util_time_update(2);      
	memset(nTemplateSize,0,5*sizeof(int32_t));
    for (i = 0; i < index_length; i++) {
		pTemplateBuffer[i] = gFingerprintVar.enrolled_template[i].fp_template_data;	
        if (sql_load_fingerprint(dbName,gFingerprintVar.gid,index[i],pTemplateBuffer[i], &nTemplateSize[i]) == 0) {			
            nTemplateSize[i] -= 1;
			gFingerprintVar.enrolled_template[i].fp_template_size = nTemplateSize[i];
        }
    	LOGD("fp_template_size=%d",gFingerprintVar.enrolled_template[i].fp_template_size);
    }

    btl_util_time_diffnow("sql",2);
#endif
    /*add for match*/
    g_fid_subindex = 0;
	btl_core_reportkey_ctl(0);
    do {   
        retWait = 0;
        //btl_api_setInterruptMode();
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
        read_image_and_cal_dacp();
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
				LOGD("%s, template size:%d", __func__, template_size);
                sql_insert_fingerprint(dbName, gFingerprintVar.gid, gFingerprintVar.fid, pTemplateBuf, template_size);
                gFingerprintVar.is_need_reload_templates = 1;
				memset(dbBackupName, 0, PATH_SIZE);
    			btl_api_getDbFilename(dbBackupName, gFingerprintVar.gid, 0);
				sql_backup_database(dbName, dbBackupName);
				#endif
                //btl_api_setCancel(1);
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
                msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_DUPLICATE_AREA;
                LOGD("FINGERPRINT_ACQUIRED_DUPLICATE_AREA samples_remaining :%d", msg.data.enroll.samples_remaining);
                dev->notify(&msg);
                LOGD("Fp_Enroll radio = lastradio :%d", lastRadio);
                continue;
            }
            //check if finger up
            btl_api_isFingerUp(g_enroll_ck_fingerup_timeout);
        } else if(enrollStatus==-ERR_AGC || enrollStatus==-ERR_EMPTY){ 
            msg.type = FINGERPRINT_ACQUIRED;
            msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_BAD_IMAGE;
            dev->notify(&msg);
            errorCounts = 0;
			continue;
        } else if(enrollStatus == -ERR_ENROLL) {
            msg.type = FINGERPRINT_ACQUIRED;
            msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_ERR_ENROLL;
            dev->notify(&msg);
            errorCounts = 0;
            continue;
        } else if(enrollStatus == -ERR_CANCEL) {
            break;
        }else if(enrollStatus == -ERR_EXIST) {
                memset(&msg, 0, sizeof(fingerprint_msg_t));
                msg.type = FINGERPRINT_ACQUIRED;
                msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_DUPLICATE_FINGER;
                LOGD("FINGERPRINT_ACQUIRED_DUPLICATE_FINGER samples_remaining :%d", msg.data.enroll.samples_remaining);
                dev->notify(&msg);
                LOGD("Fp_Enroll radio = lastradio :%d", lastRadio);
                continue;
            } 
        
        else
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
      
    if(g_shotkey_disable != 1 && gFingerprintVar.cancel != 1) {
        LOGD("%s,check fp up for shotkey",__func__);
        btl_api_isFingerUp(g_match_ck_fingerup_timeout);
    }
    btl_api_setCancel(1);
    if (btl_core_getMteStatus())
        btl_core_unInitAlgo();

    usleep(300000);			//500ms
	btl_core_reportkey_ctl(1);
    btl_api_setInterruptMode();
	free((void *)pTemplateBuf);
    pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
    LOGD("-- %s", __func__);
    return ((void *)0);
}

int match_times = 0;
#if BL_QUICK_WAKEUP_EN
#include "fingerprint_semaphore.h"
fingerprint_semaphore_t display_sem;
#endif

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
    char dbName[PATH_SIZE];
	char dbBackupName[PATH_SIZE];
    fingerprint_msg_t msg;
    fingerprint_device_t *dev;
    fingerprint_param_t  *param;
    int32_t retWait = 0;
	int32_t i;

    uint8_t* pTemplateBuffer[5];
	int32_t  nTemplateSize[5];
	if(gFingerprintVar.cancel == 1)
		return 0;
	pthread_detach(pthread_self());
    LOGD("++ %s", __func__);
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
	btl_core_reportkey_ctl(2);
    param = (fingerprint_param_t *)arg;
    dev = param->dev;
    gFingerprintVar.gid  = param->gid;
    gFingerprintVar.cancel = 0;
    memset(&msg, 0, sizeof(fingerprint_msg_t));
    btl_api_getDbFilename(dbName, gFingerprintVar.gid, 1);
	#if FINGER_DATA_FILE
	index_length = btl_file_fingerNum(index);
	LOGD("%s len:%d", __func__, index_length);
	for(i=0;i<index_length; i++)
		LOGD("index[%d] = %d", i, index[i]);
	#else
    index_length = sql_inquire_fingerpint_fid(dbName, index);
	if(index_length == -1) {
        memset(dbBackupName, 0, PATH_SIZE);
		btl_api_getDbFilename(dbBackupName, gFingerprintVar.gid, 0);
		sql_backup_database(dbBackupName, dbName);
		index_length = sql_inquire_fingerpint_fid(dbName, index);
    }
    LOGD("%s, gid:%d, len:%d", __func__, gFingerprintVar.gid, index_length);
    #endif
    
	gFingerprintVar.captured_image = malloc(g_width * g_height);
	if (gFingerprintVar.captured_image == 0){
		LOGE("Can't allocate buffer");		
        pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
        return ((void *)0);
	}	
	if(is_first_enter == 1)
	{
		is_first_enter = 0;
		for (i = 0; i < 5; i++){
			gFingerprintVar.enrolled_template[i].fp_template_data = malloc(TEMPLATE_SIZE);
			gFingerprintVar.enrolled_template[i].fp_template_size = 0;
			gFingerprintVar.enrolled_template[i].fp_template_type = 0;
		}
	}
	
	btl_util_time_update(2);   
	if(gFingerprintVar.is_need_reload_templates == 1 )
	{  
		gFingerprintVar.is_need_reload_templates = 0;
		memset(nTemplateSize,0,5*sizeof(int32_t));
		for (i = 0; i < index_length; i++) {
			pTemplateBuffer[i] = gFingerprintVar.enrolled_template[i].fp_template_data;	
		    if (sql_load_fingerprint(dbName,gFingerprintVar.gid,index[i],pTemplateBuffer[i], &nTemplateSize[i]) == 0) {			
		        nTemplateSize[i] -= 1;
				gFingerprintVar.enrolled_template[i].fp_template_size = nTemplateSize[i];
		    }
			LOGD("fp_template_size=%d",gFingerprintVar.enrolled_template[i].fp_template_size);
		}
	}
    btl_util_time_diffnow("sql",2);
	
    while(gFingerprintVar.cancel != 1 && index_length != 0) {
        //btl_api_setInterruptMode();
        btl_api_waitSignal(0xff,0);
        if (gFingerprintVar.cancel)break;
		read_image_and_cal_dacp();
        LOGD("Finger is pressed");
        memset(&msg, 0, sizeof(fingerprint_msg_t));
        msg.type = FINGERPRINT_ACQUIRED;
        msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_GOOD;
        dev->notify(&msg);
        #if BL_QUICK_WAKEUP_EN
		fingerprint_sem_post(&display_sem);
		#endif
        fingerID = 0;
        LOGD("before match");
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
			match_times++;
            LOGD("Fp_Authenticate fingerid:%d:%d", msg.data.authenticated.finger.fid, g_match_full);
			LOGD("match_times:%d", match_times);
			if(gFingerprintVar.enrolled_template[gFingerprintVar.matched_index].fp_template_size > (TEMPLATE_SIZE * 19 / 20))
			{
			LOGD("matched finger ,template is already too big:%d,UPDATE_AREA=%d", gFingerprintVar.enrolled_template[gFingerprintVar.matched_index].fp_template_size,UPDATE_AREA);
				UPDATE_AREA = 95;
				//break;
			}
			if (g_online_update && g_match_full){
			   gFingerprintVar.dyn_update_data.captured_image = malloc(g_width * g_height);
			   gFingerprintVar.dyn_update_data.matched_template_data = malloc(TEMPLATE_SIZE);
			   gFingerprintVar.dyn_update_data.fid = fingerID;
			   gFingerprintVar.dyn_update_data.uid = gFingerprintVar.gid;
			   if (gFingerprintVar.dyn_update_data.captured_image && gFingerprintVar.dyn_update_data.matched_template_data  ){
			   	  memcpy(gFingerprintVar.dyn_update_data.captured_image,gFingerprintVar.captured_image,g_width * g_height);
				  memcpy(gFingerprintVar.dyn_update_data.matched_template_data,
				  	     gFingerprintVar.enrolled_template[gFingerprintVar.matched_index].fp_template_data,
				  	     gFingerprintVar.enrolled_template[gFingerprintVar.matched_index].fp_template_size);
				  gFingerprintVar.dyn_update_data.matched_template_size = 
				  	                            gFingerprintVar.enrolled_template[gFingerprintVar.matched_index].fp_template_size;
  			      //pthread_create(&gFingerprintVar.tid, NULL, &thUpdateFunc, &gFingerprintParam);	
				  thUpdateFunc((void*)&gFingerprintParam);			  
			   	}
			}
			btl_api_isFingerUp(g_match_ck_fingerup_timeout);
            break;
        } else if(authenticateStatus == -ERR_EMPTY || authenticateStatus == -ERR_AGC ) {
            if (authenticateStatus == -ERR_AGC) {
                memset(&msg, 0, sizeof(fingerprint_msg_t));
                msg.type = FINGERPRINT_ACQUIRED;
                msg.data.acquired.acquired_info = FINGERPRINT_ACQUIRED_PARTIAL; //FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
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
                //break;
            }
            btl_api_isFingerUp(g_match_ck_fingerup_timeout);
        }
    }
    if(g_shotkey_disable != 1 && gFingerprintVar.cancel != 1) {
        LOGD("%s,check fp up for shotkey",__func__);
        btl_api_isFingerUp(g_match_ck_fingerup_timeout);
    }
	btl_core_reportkey_ctl(1);
    btl_api_setInterruptMode();
	free(gFingerprintVar.captured_image);
	gFingerprintVar.captured_image = 0;
	/*
	for (i = 0; i < 5; i++){
	   if (gFingerprintVar.enrolled_template[i].fp_template_data){
	     free(gFingerprintVar.enrolled_template[i].fp_template_data);
		 gFingerprintVar.enrolled_template[i].fp_template_data = 0;
	    }		 
		gFingerprintVar.enrolled_template[i].fp_template_size = 0;
		gFingerprintVar.enrolled_template[i].fp_template_type = 0;
	}
	*/
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
	gFingerprintVar.is_need_reload_templates = 1;
	is_first_enter = 1;
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
    //gFingerprintVar.cancel = 1;
    pthread_cond_signal(&gFingerprintVar.count_nonzero);
	while(pthread_mutex_trylock(&gFingerprintVar.fingerprint_lock) != 0){
		btl_api_setCancel(1);
		pthread_cond_signal(&gFingerprintVar.count_nonzero);
	}
   //pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    //btl_api_setInterruptMode();
    pthread_mutex_unlock(&gFingerprintVar.fingerprint_lock);
#if BTL_ANDROID_N
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
	if (btl_core_getMteStatus())
		btl_core_unInitAlgo();  
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
	if (btl_core_getMteStatus())
		btl_core_unInitAlgo(); 
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
	char dbBackupName[PATH_SIZE];
	int count = 0;
    LOGD("++ %s", __func__);
    gFingerprintVar.gid  = gid;
    strcpy(mWorkedPath, store_path);
    LOGD("%s,%s", __func__, store_path);
    btl_api_getDbFilename(dbName, gFingerprintVar.gid, 1);
    LOGD("%s,%s", __func__, dbName);
    btl_api_createDB(dbName);
	btl_api_createFile();

	memset(dbBackupName, 0, PATH_SIZE);
	btl_api_getDbFilename(dbBackupName, gFingerprintVar.gid, 0);
	count = sql_inquire_all_fingerprint(dbName);
	if(count == -1){
		sql_backup_database(dbBackupName, dbName);
	}else{
		sql_backup_database(dbName, dbBackupName);
	}

	
    LOGD("-- %s", __func__);
    return 0;
}



uint64_t Fp_PreEnroll(struct fingerprint_device __unused *dev)
{
    LOGD("++ %s", __func__);
    char dbName[PATH_SIZE];
	char dbBackupName[PATH_SIZE];
	int count = 0;
    btl_api_getDbFilename(dbName, gFingerprintVar.gid, 1);
	count = sql_inquire_all_fingerprint(dbName);

	if(count == -1){
		memset(dbBackupName, 0, PATH_SIZE);
		btl_api_getDbFilename(dbBackupName, gFingerprintVar.gid, 0);
		sql_backup_database(dbBackupName, dbName);
		count = sql_inquire_all_fingerprint(dbName);
	}

    if(count == MAX_FINGER_NUMS) 
		return 0;

    btl_api_setCancel(0);
    gFingerprintVar.token = btl_api_createToken();
    LOGD("-- %s", __func__);
    return gFingerprintVar.token;
}


int32_t Fp_Remove(fingerprint_device_t __unused *dev, uint32_t __unused gid,
                  uint32_t __unused fid)
{
    char dbFilename[PATH_SIZE];
	char dbBackupName[PATH_SIZE];
    int32_t i ;
    int32_t fids_index[10];
    int32_t fids_size = 0;
    fingerprint_msg_t msg;


    LOGD("++ %s", __func__);
    btl_api_setCancel(1);
    pthread_cond_signal(&gFingerprintVar.count_nonzero);
    pthread_mutex_lock(&gFingerprintVar.fingerprint_lock);
    LOGD("Fp_Remove:%d,%d", fid, gid);
    btl_api_getDbFilename(dbFilename, gid, 1);
    
#if (defined FINGER_DATA_FILE)
        fids_size = btl_get_fid_size(fids_index);
#else
        fids_size = sql_inquire_fingerpint_fid(dbFilename, fids_index);
#endif
        LOGD("%s, gid:%d, len:%d", __func__, gFingerprintVar.gid, fids_size);
    if (fid > 0) {
        for(i = 0; i < fids_size; i++) {
            if((int32_t)fid == fids_index[i])
            {
            	LOGD("%s, fid:%d", __func__, fid);
				#if FINGER_DATA_FILE
				file_delete_fingerprint(fid);
				#else
				if(sql_delete_fingerprint(dbFilename, gid, fid) == -1){			
					memset(dbBackupName, 0, PATH_SIZE);
					btl_api_getDbFilename(dbBackupName, gid, 0);
					sql_backup_database(dbBackupName, dbFilename);	
					sql_delete_fingerprint(dbFilename, gid, fid);
				}    
				#endif
            }
        }
			memset(&msg, 0, sizeof(fingerprint_msg_t));
			msg.type = FINGERPRINT_TEMPLATE_REMOVED;
			msg.data.removed.finger.gid = gid;
			msg.data.removed.finger.fid = fid;
			dev->notify(&msg);
    } else if (fid == 0) {
        LOGD("Fp_Remove:%d,%d", fid, gid);

        for(i = 0; i < fids_size; i++) {
            fid = fids_index[i];
            LOGD("%s, fid:%d", __func__, fid);
#if (defined FINGER_DATA_FILE)
            file_delete_fingerprint(fid);
#else
			if(sql_delete_fingerprint(dbFilename, gid, fid) == -1){			
				memset(dbBackupName, 0, PATH_SIZE);
				btl_api_getDbFilename(dbBackupName, gid, 0);
				sql_backup_database(dbBackupName, dbFilename);	
				sql_delete_fingerprint(dbFilename, gid, fid);
			}
#endif
            memset(&msg, 0, sizeof(fingerprint_msg_t));
            msg.type = FINGERPRINT_TEMPLATE_REMOVED;
            msg.data.removed.finger.gid = gid;
            msg.data.removed.finger.fid = fid;
            dev->notify(&msg);
        }
    }

	memset(&msg, 0, sizeof(fingerprint_msg_t));
    msg.type = FINGERPRINT_TEMPLATE_REMOVED;
    msg.data.removed.finger.gid = gid;
    msg.data.removed.finger.fid = 0;
    dev->notify(&msg);
#if 0//BTL_ANDROID_N
    memset(&msg, 0, sizeof(fingerprint_msg_t));
    msg.type = FINGERPRINT_ERROR;
    msg.data.error = 5 ;
    dev->notify(&msg);
#endif
    btl_api_setCancel(0);
    gFingerprintVar.is_need_reload_templates = 1;
	memset(dbBackupName, 0, PATH_SIZE);
	btl_api_getDbFilename(dbBackupName, gid, 0);
	sql_backup_database(dbFilename, dbBackupName);	

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
    btl_api_getDbFilename(dbFilename, gFingerprintVar.gid, 1);
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
    btl_api_getDbFilename(dbFilename, gFingerprintVar.gid, 1);
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
