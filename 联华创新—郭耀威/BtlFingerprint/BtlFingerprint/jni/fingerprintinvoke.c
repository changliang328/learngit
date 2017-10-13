#include <jni.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "bl29xxfinger.h"
#include <android/log.h>

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "btl_jni",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "btl_jni",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "btl_jni",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "btl_jni",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "btl_jni",__VA_ARGS__)


int FpWaitScreenOn(JNIEnv* env, jobject thiz) {
    return User_WaitScreenOn();
}

int FpIsFingerUp(JNIEnv* env, jobject thiz) {
    return  User_IsFingerUp();
}


int FpPowerDown(JNIEnv* env, jobject thiz) {
    return User_PowerDown();
}

int FpWriteKeycode(JNIEnv* env, jobject thiz,jint keycode) {
    return User_WriteKeycode(keycode);
}

int FpGetFingerID(JNIEnv* env, jobject thiz, jintArray result_buf ) {
	LOGD("FpSettingMode");
	jint * pResult = (*env)->GetIntArrayElements(env, result_buf, 0);
	User_GetId(pResult);
	(*env)->ReleaseIntArrayElements(env, result_buf, pResult, 0);
	return 0;
}

/*JNI register*/
/*尤其要注意这里的命名方式：包名/类名  ！！！！*/
static const char *classPathName = "com/btlfinger/fingerprint/FpNative";

static JNINativeMethod methods[] = {
    {"FpWaitScreenOn", "()I", (void*)FpWaitScreenOn },
    {"FpIsFingerUp", "()I", (void*)FpIsFingerUp },
    {"FpPowerDown", "()I", (void*)FpPowerDown },
	{"FpWriteKeycode", "(I)I", (void*)FpWriteKeycode },
	{"FpGetFingerID", "([I)I",  (void*)FpGetFingerID},
};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,JNINativeMethod* gMethods, int numMethods) {
    jclass clazz;

    clazz = (*env)->FindClass(env, className);
    if (clazz == NULL) {
        //LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        //LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 *
 * returns JNI_TRUE on success.
 */
static int registerNatives(JNIEnv* env) {
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
    
    //LOGI("JNI_OnLoad");
    
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
