#include <jni.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#include "bf_log.h"
#include <android/log.h>
#include "bf_hal.h"
#include "bf_fingerprint.h"
#include "bf_types.h"
struct bf_fingerprint_hal_device_t *g_bf_hal_fp_dev;
bf_ca_app_data_t g_fpdata;

int32_t bl_fp_Init()
{
	BF_LOG("+++");
	bf_fingerprint_open(NULL, NULL, (hw_device_t**)&g_bf_hal_fp_dev);
	BF_LOG("---");
	return 0;
}
int32_t bl_fp_UnInit()
{
	struct bf_fingerprint_hal_device_t *dev = g_bf_hal_fp_dev;
	mmi_info *pMmiInfo = &dev->tMmiInfo;
	BF_LOG("+++");
	bf_fingerprint_close((hw_device_t *)g_bf_hal_fp_dev);
	BF_LOG("---");
	return 0;
}

int32_t bl_fp_get_fingerdata()
{
	bf_get_finger_data(&g_fpdata);
	return 0;
}

int32_t bl_fp_GetRawImage(uint8_t * pBmp, int32_t *params, int32_t *result)
{
	uint32_t width = g_fpdata.width;
	uint32_t height = g_fpdata.height;
	struct bf_fingerprint_hal_device_t *dev = g_bf_hal_fp_dev;
	mmi_info *pMmiInfo = &dev->tMmiInfo;
	mmi_test_params_t *pmmi_params = (mmi_test_params_t *)dev->imagebuf;
	uint32_t *pudacp = (uint32_t *)&pmmi_params->params[0];
	dev->workstate = 0;
	pMmiInfo->testType = MMI_TYPE_GET_RAWIMAGE_WITHOUT_INT;
	pMmiInfo->mmi_data = pmmi_params;
	
	pmmi_params->cmd = BF_CMD_DO_MMI_TEST;
	pmmi_params->action = BF_MMI_ACT_CAPTURE_WITH_DACP;
	pmmi_params->length = width * height;
	*pudacp = params[0];
	BF_LOG("pmmi_params->params[0]%d,pMmiInfo->isTesting=%d+++",pmmi_params->params[0],pMmiInfo->isTesting);
	
	bf_start_mmi_test(dev);
	memcpy(pBmp, pmmi_params->params, height * width);
	BF_LOG("---");
	return 0;
}

int FpInit(JNIEnv* env,jobject thiz, jobject obj) {
	jclass cls;
	jfieldID fid;	/* store the field ID */
	if(bl_fp_Init())
		return -1;
	bl_fp_get_fingerdata();

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
