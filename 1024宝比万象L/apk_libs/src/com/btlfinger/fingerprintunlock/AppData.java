package com.btlfinger.fingerprintunlock;

import com.btlfinger.fingerprint.dao.FpsTable;
import com.btlfinger.fingerprint.FingerPrintManager;
import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.util.Log;
import com.btlfinger.fingerprint.utils.CrashHandlerUtils;
import com.btlfinger.fingerprint.FpNative;
/**
 * @author blestech
 * @since 2015-11-26
 */
public class AppData extends Application {
	private static Context sContext;
	private static int FPICID = 20867;
	@Override
	public void onCreate() {
		super.onCreate();
				
		int [] finger_id = new int[10];
		FpNative mFpNative = new FpNative();
		mFpNative.FpGetFingerID(finger_id);
		Log.i("AppData", "finger_id[0]: "+ finger_id[0]);
		if(finger_id [0] != FPICID){
			Log.i("AppData", "matchFingerID fail: "+ finger_id[0]);
			//System.exit(0);
		}
		
		sContext = getApplicationContext();
		chmodDir();

		//Settings.System.putString(getContentResolver(), PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME,
		//		PackagesConstant.FINGERPRINTUNLCOK_PACKAGENAME);
		 /**
	     * 在这里处理异常
	     */
	    CrashHandlerUtils crashHandler = CrashHandlerUtils.getInstance();    
	    /**
	     * 注册crashhandler    
	     */
	    crashHandler.init(getApplicationContext());    
	}

	public static Context getContext() {
		return sContext;
	}


	private int safe_lock = FingerPrintManager.MSG_RES.MSG_REG_OK;

	public int getSafe_lock() {
		return safe_lock;
	}

	public void setSafe_lock(int safe_lock) {
		this.safe_lock = safe_lock;
	}

	/*
	 * 当前数据库中已保存的指纹个数
	 */
	private int finger_count = 0;

	public int getFinger_count() {

		return finger_count;
	}

	public void setFinger_count(int finger_count) {
		this.finger_count = finger_count;
	}

	public static void  chmodDir() {
		String filenameString = FingerPrintManager.FP_DATA_DIR;
		String command = "chmod 777 " + filenameString;// 全部权限
		Runtime runtime = Runtime.getRuntime();
		try {
			Process proc = runtime.exec(command);
		} catch (Exception e) {
		}

	}

}
