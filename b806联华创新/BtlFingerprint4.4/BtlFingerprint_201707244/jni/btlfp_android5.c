/*
 * Copyright (C) 2016 BetterLife.Co.Ltd. All rights  reserved.
 *
 */
#include <jni.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "btlfp.h"
#include <android/log.h>



const char *fpDataPath = "/data/data/com.btlfinger.fingerprintunlock/";


/*JNI register*/
static const char *classPathName = "com/btlfinger/fingerprint/FpNative";


static void btl_android_getDbFilename(char *dbFilename)
{
    const char *dbName = "fp.db";
    LOGD("++ %s", __func__);
    strcpy(dbFilename, fpDataPath);
    strcat(dbFilename, dbName);
    LOGD("%s", dbFilename);
    LOGD("-- %s", __func__);
}

static int32_t btl_android_createDB(char *dbName)
{
    sql_create_database(dbName);
    return 0;
}

static int32_t btl_android_getFpData(uint8_t *pTemplateBuf, uint32_t *pSize, int fid)
{
    char tempBuf[PATH_SIZE];
    LOGD("++ %s", __func__);
    LOGD("%s:%d", __func__, fid);
    btl_core_getTemplate(pTemplateBuf, pSize);
    LOGD("-- %s", __func__);
    return 0;
}


int32_t btl_android_removeFp(uint32_t fid)
{
    char dbFilename[PATH_SIZE];
    int32_t i ;
    int32_t fids_index[10];
    int32_t fids_size = 0;

    LOGD("++ %s", __func__);
    btl_android_getDbFilename(dbFilename);
    if (fid > 0) {
		#if FINGER_DATA_FILE
		file_delete_fingerprint(fid);
		#else
        sql_delete_fingerprint(dbFilename, 0, fid);    
		#endif
/*	//davie		
        sql_delete_fingerprint(dbFilename, 0, fid);
*/        
    } else if (fid == 0) {
        LOGD("Fp_Remove:%d,%d", fid, 0);
        fids_size = sql_inquire_fingerpint_fid(dbFilename, fids_index);
        LOGD("%s, len:%d", __func__, fids_size);
        for(i = 0; i < fids_size; i++) {
            fid = fids_index[i];
            LOGD("%s, fid:%d", __func__, fid);
            sql_delete_fingerprint(dbFilename, 0, fid);
        }
    }
    LOGD("-- %s", __func__);
    return 0;
}
/**
 *
 */
int FpInitRegister(JNIEnv* env,jobject thiz)
{

    return btl_core_InitAlgo();
}

/**
 *
 */
int FpRecordFingerprint(JNIEnv* env,jobject thiz)
{
    int radio;
    int tezhen_flag;
    LOGD("%s++",__func__);
    btl_core_setCancel(0);
    tezhen_flag = btl_core_registeFp(&radio);
    LOGD("%s : %d ",__func__,tezhen_flag);
    LOGD("%s--",__func__);
    return tezhen_flag;
}

/**
 *
 */
int FpSaveTemplate(JNIEnv* env,jobject thiz, jint fid)
{
    char fpDBName[PATH_SIZE];
    uint32_t template_size = 0;
    uint8_t* pTemplateBuf = NULL;

    pTemplateBuf = malloc(512 * 1024);

    if(pTemplateBuf == NULL) {
        LOGE("%s, can't allocate buffer", __func__);
        return ((void *)-1);
    }
/*	//davie
    btl_android_getDbFilename(fpDBName);
*/
	LOGD("%s, fid: %d", __func__, fid);

    btl_android_getFpData(pTemplateBuf, &template_size, fid);
	#if FINGER_DATA_FILE
	fid = btl_file_fingerid();
	LOGD("%s, fid: %d", "btl_file_fingerid", fid);
	btl_file_saveTemplate(pTemplateBuf, template_size, fid);
	#else
	btl_android_getDbFilename(fpDBName);
    sql_insert_fingerprint(fpDBName, 0, fid, pTemplateBuf, template_size);
	#endif
/*	//davie
    sql_insert_fingerprint(fpDBName, 0, fid, pTemplateBuf, template_size);
*/
    free(pTemplateBuf);

    if (btl_core_getMteStatus())
        btl_core_unInitAlgo();

    return (int32_t)template_size;
}


/**
 *
 */
jintArray FpMatchFingerprint(JNIEnv*  env,jobject  thiz, jstring exist_fp_folder,jintArray fileindex,jintArray templateLength,jint length)
{

    int i;
    int fd=0;
    jint* arr;
    jint sum;
    char fpDBName[PATH_SIZE];
    int myIndex = -1;
    int res_match;

    LOGD("%s++",__func__);

    jint * fileArray = (*env)->GetIntArrayElements(env, fileindex, 0);

    btl_android_getDbFilename(fpDBName);

    LOGD("len:%d",length);

	#if FINGER_DATA_FILE
		length = btl_file_fingerNum(fileArray);
		LOGD("%s len:%d", __func__, length);
	#endif

    btl_core_setCancel(0);

    for (i = 0; i < length; i++)
        LOGD("id:%d",fileArray[i]);

    res_match = btl_core_matchFpC(fpDBName, fileArray, length, 0,&myIndex);

    (*env)->ReleaseIntArrayElements(env, fileindex, fileArray, 0);

    jintArray iarr = (*env)->NewIntArray(env, 2);
    int buf[2];
    buf[0] = res_match == 0? -1:res_match;
    buf[1] = myIndex;
    (*env)->SetIntArrayRegion(env, iarr, 0, 2, (const jint*) buf);

    return iarr;
}

/**
 *
 */

int FpRemoveFp(JNIEnv*  env, jobject thiz,jint fid)
{
    LOGD("%s",__func__);
    return btl_android_removeFp(fid);
}

/**
 *
 */

int FpCancelAction(JNIEnv* env, jobject thiz ,jint isRegist)
{
    LOGD("%s",__func__);
    if (btl_core_getMteStatus())
    	btl_core_unInitAlgo();
    return btl_core_setCancel(isRegist);
}
/**
 *
 */

int FpWaitScreenOn(JNIEnv* env, jobject thiz)
{
    LOGD("%s",__func__);
    return btl_core_setInterruptWorkMode();
}

/**
 *
 */
int FpIsFingerUp(JNIEnv* env, jobject thiz)
{
    LOGD("%s",__func__);
    return  btl_core_isFingerUp();
}

/**
 *
 */
int FpWriteKeycode(JNIEnv*  env, jobject thiz,jint keycode)
{
    LOGD("%s keycode :%d\n",__func__, keycode);
    return btl_hw_writeKeycode(keycode);
}

/**
 *
 */
int FpMmiFpTest(JNIEnv* env, jobject thiz)
{
    LOGD("%s",__func__);
    return btl_core_mmiTest();
}

int FpGetFingerID(JNIEnv* env, jobject thiz, jintArray result_buf )
{
    LOGD("FpSettingMode");
    jint * pResult = (*env)->GetIntArrayElements(env, result_buf, 0);
    btl_core_User_GetId(pResult);
//	*pResult = 21123;
    (*env)->ReleaseIntArrayElements(env, result_buf, pResult, 0);
    return 0;
}


static JNINativeMethod methods[] = {
    {"FpInitRegister",      "()I",  (void*)FpInitRegister },
    {"FpRecordFingerprint", "()I",  (void*)FpRecordFingerprint },
    {"FpSaveTemplate",      "(I)I", (void*)FpSaveTemplate },
    {"FpRemoveFp",          "(I)I", (void*)FpRemoveFp },
    {"FpMatchFingerprint",  "(Ljava/lang/String;[I[II)[I", (void*)FpMatchFingerprint},
    {"FpCancelAction",      "(I)I", (void*)FpCancelAction },
    {"FpWaitScreenOn",      "()I",  (void*)FpWaitScreenOn },
    {"FpIsFingerUp",        "()I",  (void*)FpIsFingerUp },
    {"FpWriteKeycode",      "(I)I", (void*)FpWriteKeycode },
    {"FpMmiFpTest",         "()I",  (void*)FpMmiFpTest },
    {"FpGetFingerID", "([I)I",  (void*)FpGetFingerID},
};


/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    char dbFilename[PATH_SIZE];


    LOGD("%s++", __func__);

    clazz = (*env)->FindClass(env, className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }

    LOGD("numMethods '%d'", numMethods);
    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    btl_android_getDbFilename(dbFilename);

    btl_android_createDB(dbFilename);

    btl_core_getDefaultParams();

    LOGD("%s--", __func__);

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static int registerNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, classPathName,methods, sizeof(methods) / sizeof(methods[0]))) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */

typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;

    if ((*vm)->GetEnv(vm, &uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        //LOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        //LOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;

bail:
    return result;
}
