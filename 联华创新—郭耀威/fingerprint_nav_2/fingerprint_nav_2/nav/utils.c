/******************** (C) COPYRIGHT 2017 BTL ********************************
* File Name          : log.h
* Author               : guoyaowei
* Version              : 1.0
* Date                  : 2017.2.21
*******************************************************************************/

#include <jni.h>

#include "log.h"
#include "nav.h"

#define TAG		"nav_util"




int btl_nav_utils_getSta(void)
{
	return btl_nav_core_ctl_getSta();
}
int btl_nav_utils_setSta(int sta)
{
	return btl_nav_core_ctl_setSta(sta);
}

#ifdef USE_PIPE_CONN
int btl_nav_utils_init(void)
{
	if(btl_nav_utils_getSta() == 0)
	{
		if(btl_nav_core_init() < 0)
			return -1;
#ifdef USE_PIPE_CONN
		if(btl_nav_core_ctl_init() < 0)
			return	-2;
#else
		
#endif
		btl_nav_core_ctl_setSta(1);
		return 0;

	}
	return 0;
}
int btl_nav_utils_uinit(void)
{
	if(btl_nav_utils_getSta() == 1)
	{
		btl_nav_core_ctl_uinit();
			return btl_nav_core_ctl_setSta(0);
	}
	return 0;
}
#else
int btl_nav_utils_init(void)
{
	if(btl_nav_core_init() < 0)
		return -1;
	btl_nav_core_ctl_setSta(1);
	return 0;
}
int btl_nav_utils_uinit(void)
{
//	btl_nav_core_ctl_uinit();

	btl_nav_core_cancel();

	return btl_nav_core_ctl_setSta(0);
}
#endif

JNIEXPORT jint JNICALL Java_com_blestech_navigation_NavCtrlJni_init
  (JNIEnv *env, jclass cls, jbyteArray byteArray)
{

	return btl_nav_utils_init();
}


JNIEXPORT jint JNICALL Java_com_blestech_navigation_NavCtrlJni_start
  (JNIEnv *env, jclass cls)
{
	return btl_nav_core_start();
}

JNIEXPORT jint JNICALL Java_com_blestech_navigation_NavCtrlJni_stop
  (JNIEnv *env, jclass cls)
{
	return btl_nav_core_stop();
}
JNIEXPORT jint JNICALL Java_com_blestech_navigation_NavCtrlJni_uinit
  (JNIEnv *env, jclass cls)
{
	return btl_nav_utils_uinit();
}
JNIEXPORT jint JNICALL Java_com_blestech_navigation_NavCtrlJni_getSta
  (JNIEnv *env, jclass cls)
{
	return btl_nav_utils_getSta();
}

JNIEXPORT jint JNICALL Java_com_blestech_navigation_NavCtrlJni_setSta
  (JNIEnv *env, jclass cls, jint sta)
{
	return btl_nav_utils_setSta(sta);
}


JNIEXPORT jint JNICALL Java_com_blestech_navigation_NavCtrlJni_simulateX
  (JNIEnv *env, jclass cls, jint pos)
{
	return btl_nav_simulateTouch("down");
}

JNIEXPORT jint JNICALL Java_com_blestech_navigation_NavCtrlJni_simulateY
  (JNIEnv *env, jclass cls, jint pos)
{
	return btl_nav_simulateTouch("up");
}


