#include <stdio.h>
#include <jni.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include "bf_mmi.h"
#include "bf_log.h"
#include "bf_types.h"

#define BF_IOCTL_MAGIC_NO			0xFB
#define BF_IOCTL_SET_WORK_STATE             _IOW(BF_IOCTL_MAGIC_NO,  14,uint32_t)
#define BF_IOCTL_GET_WORK_STATE             _IOW(BF_IOCTL_MAGIC_NO,  15,uint32_t)
int32_t bl_set_workstate(int fd, int state);
int32_t bl_get_workstate(int fd);
#define DEV_NAME BF_FP_DEV_NAME
typedef struct {
	int devfd;
	int width;
	int height;
	uint32_t capdacp;
} bf_test_data_t;
static bf_test_data_t g_fpdata = { 0 };

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

int32_t bl_fp_Init()
{
	uint32_t workstate = 0;
	BF_LOG("+++");
	g_fpdata.devfd = open(DEV_NAME, O_RDWR);
	workstate = set_workstate_cmd(workstate, MMI_TYPE_GET_HEIGHT_WIDTH);
	workstate = do_cmd_until_done(g_fpdata.devfd, workstate);
	g_fpdata.width = get_workstate_arg1(workstate);
	g_fpdata.height = get_workstate_arg2(workstate);
	BF_LOG("devfd=%d,width=%d,height=%d", g_fpdata.devfd, g_fpdata.width,
			g_fpdata.height);
	BF_LOG("---");
	return 0;
}
int32_t bl_fp_UnInit()
{
	BF_LOG("+++");
	close(g_fpdata.devfd);
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

int32_t bl_fp_GetRawImage(uint8_t * pBmp, int32_t *params, int32_t *result)
{
	uint32_t workstate = 0;
	BF_LOG("++++");
	workstate = set_workstate_cmd(workstate, MMI_TYPE_GET_RAWIMAGE_WITHOUT_INT);
	workstate = set_workstate_arg1(workstate, params[0]);
	workstate = do_cmd_until_done(g_fpdata.devfd, workstate);
	pBmp[0] = 0xff; //for get rawimage from driver
	read(g_fpdata.devfd, pBmp, g_fpdata.width * g_fpdata.height);
	BF_LOG("-----");
	return 0;
}

int FpInit(JNIEnv* env,jobject thiz, jobject obj) {
	jclass cls;
	jfieldID fid;	/* store the field ID */
	if(bl_fp_Init())
		return -1;

	cls = (*env)->GetObjectClass(env, obj);

	fid = (*env)->GetFieldID(env, cls, "width", "I");
	if (fid == NULL) {
		return -2; /* failed to find the field */
	}
	//jint width = (*env)->GetIntField(env, cls, fid);
	(*env)->SetIntField(env, obj, fid, g_fpdata.width);

	fid = (*env)->GetFieldID(env, cls, "height", "I");
	if (fid == NULL) {
		return -2; /* failed to find the field */
	}
	(*env)->SetIntField(env, obj, fid, g_fpdata.height);

	fid = (*env)->GetFieldID(env, cls, "capdacp", "I");
	if (fid == NULL) {
		return -2; /* failed to find the field */
	}
	(*env)->SetIntField(env, obj, fid,  g_fpdata.capdacp);
	LOGD("FpInit width %d, height:%d, capdacp:%d\n",
		g_fpdata.width, g_fpdata.height, g_fpdata.capdacp);
	return 0;
}

int FpUninit(JNIEnv* env,jobject thiz) {
	LOGD("FpUninit ");
	return bl_fp_UnInit();
}


int FpGetFingerRawImage(JNIEnv* env, jobject thiz, jbyteArray image_buf,jintArray params_buf, jintArray result_buf ) {
	jbyte * pBmp = (*env)->GetByteArrayElements(env, image_buf, 0);
	jint * pParams = (*env)->GetIntArrayElements(env, params_buf, 0);
	jint * pResult = (*env)->GetIntArrayElements(env, result_buf, 0);
	//LOGD("FpGetFingerRawImage paultest %x",(uint32_t)pBmp);
	int32_t ret = bl_fp_GetRawImage((uint8_t *)pBmp,(int32_t *)pParams,(int32_t *)pResult);

	(*env)->ReleaseByteArrayElements(env, image_buf, pBmp, 0);
	(*env)->ReleaseIntArrayElements(env, params_buf, pParams, 0);
	(*env)->ReleaseIntArrayElements(env, result_buf, pResult, 0);
	return ret;
}

/*JNI register*/
/*尤其要注意这里的命名方式：包名/类名  ！！！！*/
static const char *classPathName = "com/betterlife/fingerprint/FpNative";
static JNINativeMethod methods[] = {
    {"FpInit", "(Lcom/betterlife/fingerprint/FingerprintData;)I", (void*)FpInit },
    {"FpUninit", "()I", (void*)FpUninit },
    {"FpGetWorkstate", "()I", (void*)FpGetWorkstate },
    {"FpSetWorkstate", "(I)I", (void*)FpSetWorkstate },
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
