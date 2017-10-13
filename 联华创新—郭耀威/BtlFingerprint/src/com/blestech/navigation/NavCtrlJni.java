/******************** (C) COPYRIGHT 2015 GLIMMER ********************************
* File Name          : navCtrlJni.java
* Author               : guoyaowei
* Version              : 1.0
* Date                  : 2017.2.21
*******************************************************************************/
package com.blestech.navigation;

public class NavCtrlJni{
	static{
		System.loadLibrary("navgation");	
	}
	

	static public native int init(byte[] databuf);
	
	static public native int start();

	static public native int stop();
	
	static public native int pause();
	
	static public native int resume();
	
	static public native int uinit();
	
	static public native int getSta();
	
	static public native int setSta(int sta);
}