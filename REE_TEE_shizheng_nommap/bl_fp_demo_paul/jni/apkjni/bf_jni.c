#include <stdio.h>
#include <jni.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include "bf_mmi.h"
#include "com_betterlife_fingerprint_FpNative.h"
#define LOG_TAG "paulapk"
#include <android/log.h>
#define BF_LOG(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG,"%5d: <%s>" fmt "\n",  __LINE__, __func__, ##args)

#define BF_IOCTL_MAGIC_NO			0xFB
#define BF_IOCTL_SET_WORK_STATE             _IOW(BF_IOCTL_MAGIC_NO,  14,uint32_t)
#define BF_IOCTL_GET_WORK_STATE             _IOW(BF_IOCTL_MAGIC_NO,  15,uint32_t)
int32_t bl_set_workstate(int fd, int state);
int32_t bl_get_workstate(int fd);

typedef struct {
	int width;
	int height;
	int devfd;
} bf_test_data_t;
static bf_test_data_t gData = { 0 };
#define DEV_NAME "/dev/blestech_fp"
#include "bf_mmi.h"
static int do_cmd_until_done(int fd, uint32_t state) {
	uint32_t tempstate = state;
	int count = 500; //50*100=5000ms, 5S timeout
	bl_set_workstate(fd, state);
	while ((tempstate == state) && (count-- > 0)) {
		usleep(10 * 1000);
		tempstate = bl_get_workstate(fd);
	}
	return tempstate;
}

int32_t bl_fp_Init() {
	uint32_t workstate = 0;
	BF_LOG("+++");
	gData.devfd = open(DEV_NAME, O_RDWR);
	workstate = set_workstate_cmd(workstate, MMI_TYPE_GET_HEIGHT_WIDTH);
	workstate = do_cmd_until_done(gData.devfd, workstate);
	gData.width = get_workstate_arg1(workstate);
	gData.height = get_workstate_arg2(workstate);
	BF_LOG("---devfd=%d,width=%d,height=%d", gData.devfd, gData.width,
			gData.height);
	return 0;
}
int32_t bl_fp_UnInit() {
	BF_LOG("+++");
	close(gData.devfd);
	BF_LOG("---");
	return 0;
}

int32_t bl_set_workstate(int fd, int state) {
	int ret = 0;

	if (ioctl(fd, BF_IOCTL_SET_WORK_STATE, (unsigned long) &state) < 0) {
		BF_LOG("bl_set_workstate failed");
		ret = -1;
	} else {
		ret = 1;
		//BF_LOG("bl_set_workstate ok");
	}
	BF_LOG("bl_set_workstate=%x,ret=%d", state, ret);
	return ret;
}

int32_t bl_get_workstate(int fd) {
	int ret = 0;
	int state;

	if (ioctl(fd, BF_IOCTL_GET_WORK_STATE, (unsigned long) &state) < 0) {
		BF_LOG("bl_get_workstate failed");
		ret = -1;
	} else {
		ret = 1;
		//BF_LOG("bl_get_workstate ok");
	}
	BF_LOG("bl_get_workstate=%x", state);
	return state;
}

JNIEXPORT jint JNICALL Java_com_betterlife_fingerprint_FpNative_FpInit(
		JNIEnv *env, jclass cls) {
	return bl_fp_Init();
}

/*
 * Class:     com_betterlife_fingerprint_FpNative
 * Method:    FpUninit
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_betterlife_fingerprint_FpNative_FpUninit(
		JNIEnv *env, jclass cls) {
	return bl_fp_UnInit();
}

/*
 * Class:     com_betterlife_fingerprint_FpNative
 * Method:    FpSetWorkstate
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_betterlife_fingerprint_FpNative_FpSetWorkstate(
		JNIEnv *env, jclass cls, jint workstate) {
	return bl_set_workstate(gData.devfd, workstate);
}

/*
 * Class:     com_betterlife_fingerprint_FpNative
 * Method:    FpGetWorkstate
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_betterlife_fingerprint_FpNative_FpGetWorkstate(
		JNIEnv *env, jclass cls) {
	return bl_get_workstate(gData.devfd);
}

jint JNICALL Java_com_betterlife_fingerprint_FpNative_FpAutoTest(JNIEnv *env,
		jclass cls) {
	uint32_t workstate = 0;
	uint32_t result = 0;
	workstate = set_workstate_cmd(workstate, MMI_TYPE_AUTO_TEST);
	workstate = do_cmd_until_done(gData.devfd, workstate);
	result = get_workstate_result(workstate) ;

	return (BF_MMI_TEST_OK == result)?0:1 ;
}

/*
 * Class:     com_betterlife_fingerprint_FpNative
 * Method:    FpGetFingerRawImage
 * Signature: ([B[I[I)I
 */
JNIEXPORT jint JNICALL Java_com_betterlife_fingerprint_FpNative_FpGetFingerRawImage(
		JNIEnv *env, jclass cls, jbyteArray image_buf, jintArray params_buf,
		jintArray result_buf) {
	uint32_t workstate = 0;
	BF_LOG("++++");
	jbyte * pBmp = (*env)->GetByteArrayElements(env, image_buf, 0);
	workstate = set_workstate_cmd(workstate, MMI_TYPE_GET_RAWIMAGE_WITHOUT_INT);
	workstate = set_workstate_arg1(workstate, 140);
	workstate = do_cmd_until_done(gData.devfd, workstate);
	pBmp[0] = 0xff; //for get rawimage from driver
	BF_LOG("-----");
	read(gData.devfd, pBmp, gData.width * gData.height);
	(*env)->ReleaseByteArrayElements(env, image_buf, pBmp, 0);
}

/*JNI register*/
/*尤其要注意这里的命名方式：包名/类名  ！！！！*/
static const char *classPathName = "com/betterlife/fingerprint/FpNative";
static JNINativeMethod methods[] = {
    {"FpInit", "(Lcom/betterlife/fingerprint/FingerprintData;)I", (void*)FpInit },
    {"FpUninit", "()I", (void*)FpUninit },
    {"FpGetFingerRawImage", "([B[I[I)I", (void*)FpGetFingerRawImage },
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
