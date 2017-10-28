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

    /**
     * 初始化工作
     */
    public native int FpInitRegister();

    /**
     * 
     */
    public native int FpRecordFingerprint();
    /**
     *
     */
    public native int FpSaveTemplate(int fid);
    /**
     *
     */
	public native int FpRemoveFp(int fid);
    
    /**
     * 
     * @param exist_fp_folder 保存指纹数据文件的目录
     * @param index 文件
     * @return 对比结果：-1为失败
     */
    public native int[] FpMatchFingerprint(String exist_fp_folder,int[] index,int[] templateLength,int length);
    
    /**
     * 取消操作
     */
    public native int FpCancelAction(int isRegist);
     
   /**
    *
    */
    public native int FpIsFingerUp();

   /**
    *
    */
    public native int FpWaitScreenOn();
   /**
    *
    */
   
    public native int FpWriteKeycode(int keyCode);		
   /**
    *
    */
   public native int FpMmiFpTest();
   public native int FpGetFingerID(int[] result);
}

