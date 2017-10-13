package com.btlfinger.fingerprint;

/**
 * native 方法
 *
 * @author blestech
 * @since 2015-11-26
 */
public class FpNative {
    static {
      System.loadLibrary("btlfp");
    	
    }
    
    public native int FpIsFingerUp();
    /**
     *进入中断模式，使用指纹唤醒屏幕
     */
    public native int FpWaitScreenOn();
    /**
     *进入powerDown模式
     */
    public native int FpPowerDown();

	public native int FpWriteKeycode(int keyCode);				
	
	public native int FpGetFingerID(int[] result);
}

