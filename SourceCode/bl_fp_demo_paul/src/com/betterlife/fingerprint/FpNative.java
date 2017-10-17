package com.betterlife.fingerprint;

/**
 * 
 * @author paul
 * @since 2017-9-5
 */
public class FpNative {
	static {
		
		System.loadLibrary("jnibtlfp");
	}
	
	/**
	 * 初始化
	 * 
	 * @return
	 */
	public native static int FpInit();

	/**
	 * 初始化
	 * 
	 * @return
	 */
	public native static int FpUninit();
	
	/**
	 * 获取指纹图像
	 * @param image
	 * @return
	 */
	public native static int FpGetFingerRawImage(byte[] image, int[] params, int[] result);
	
	
}
