package com.btlfinger.fingerprintunlock.support;

import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.provider.Settings;
import com.btlfinger.fingerprintunlock.AppData;
import com.btlfinger.fingerprintunlock.PackagesConstant;
import com.blestech.navigation.NavCtrlJni;
import com.btlfinger.fingerprint.FingerPrintManager;
/**
 * Preference工具类
 * 
 * @author blestech
 * @since 2015-11-26
 */

public class PreferenceUtils {
	public static String KEY_ISSAFETY = "key_issafety_su";
	public static String KEY_NEED_PWD = "key_need_pwd_su";
	public static String KEY_PWD = "key_pwd_su";
	public static String VLUE_ERROR_PWD = "value_error_pwd_su";
	public static String SYSTEM_SETTINGS_FP_SCREENLOCK = "com_btlfinger_fingerprint_usedto_screenlock";
	public static String SYSTEM_SETTINGS_FP_APPLOCK = "com_btlfinger_fingerprint_usedto_applock";
	public static String SYSTEM_SETTINGS_FP_CAMERALOCK = "com_btlfinger_fingerprint_usedto_camera";
	public static String SYSTEM_SETTINGS_FP_MUSICLOCK = "com_btlfinger_fingerprint_usedto_music";
	public static String SYSTEM_SETTINGS_FP_CALLLOCK = "com_btlfinger_fingerprint_usedto_call";
	public static String SYSTEM_SETTINGS_FP_DESKTOPLOCK= "com_btlfinger_fingerprint_usedto_desktop";
	public static String SYSTEM_SETTINGS_FP_READING= "com_btlfinger_fingerprint_usedto_reader";
	public static String SYSTEM_SETTINGS_FP_NAVFUNC= "com_btlfinger_fingerprint_nav_func";	//davie
	
	public static int isSafety() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(AppData.getContext());
		return preferences.getInt(KEY_ISSAFETY, 0);
	}

	public static void setSafety(int isSafe) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(AppData.getContext());
		preferences.edit().putInt(KEY_ISSAFETY, isSafe).commit();
	}

	public static boolean isNeedPwd() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(AppData.getContext());
		return preferences.getBoolean(KEY_NEED_PWD, false);
	}
	
	public static void enablePwd(boolean needPwd) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(AppData.getContext());
		preferences.edit().putBoolean(KEY_NEED_PWD, needPwd).commit();
	}

	public static void putPwd(String pwd) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(AppData.getContext());
		preferences.edit().putString(KEY_PWD, pwd).commit();
	}

	public static String getPwd() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(AppData.getContext());
		return preferences.getString(KEY_PWD, VLUE_ERROR_PWD);
	}
	/**
	 * 设置指纹解锁用于屏幕开关
	 * @param value
	 */
	public static void setSettingFpScreenLockOffOn(boolean value) {
		Settings.System.putInt(AppData.getContext().getContentResolver(),SYSTEM_SETTINGS_FP_SCREENLOCK, value == true ? 1 : 0);
		
		if(value == false /* && isSettingFpAppLockOn() == false*/) {
			//如果两者同时关闭，则进入powerdown模式
            //记得在这里进入powerdown模式
		}
	}
	
	/**
	 * 指纹用于屏幕解锁是否打开
	 */
	public static boolean isSettingFpScreenLockOn() {
		int able = Settings.System.getInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_SCREENLOCK, 0);
		return able == 1?true:false;
	}
	/**
	 * 设置拍照控制
	 * @return
	 */

	public static void setSettingFpCameraLockOffOn(boolean value) {
	    if (value == false) {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_CAMERALOCK, 0);
	    } else {
	    	Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_CAMERALOCK, 1);
	    }
	}
	public static boolean isSettingFpCameraLockOn() {
		int able = Settings.System.getInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_CAMERALOCK, 0);
		return able == 1?true:false;
	}
	
	/**
	 * 设置音乐控制
	 * @return
	 */
	
	public static void setSettingFpMusicLockOffOn(boolean value) {
		if (value == false) {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_MUSICLOCK, 0);
		} else {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_MUSICLOCK, 1);
		}
	}
	public static boolean isSettingFpMusicLockOn() {
		int able = Settings.System.getInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_MUSICLOCK, 0);
		return able == 1?true:false;
	}
	
	/**
	 * 设置接听控制
	 * @return
	 */
	
	public static void setSettingFpCallLockOffOn(boolean value) {
		if (value == false) {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_CALLLOCK, 0);
		} else {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_CALLLOCK, 1);
		}
	}
	public static boolean isSettingFpCallLockOn() {
		int able = Settings.System.getInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_CALLLOCK, 0);
		return able == 1?true:false;
	}
	
	/**
	 * 设置桌面控制
	 * @return
	 */
	
	public static void setSettingFpDesktopLockOffOn(boolean value) {
		if (value == false) {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_DESKTOPLOCK, 0);
		} else {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_DESKTOPLOCK, 1);
		}
	}
	
	public static boolean isSettingFpDesktopLockOn() {
		int able = Settings.System.getInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_DESKTOPLOCK, 0);
		return able == 1?true:false;
	}
	/**
	 * 设置看书翻页
	 * @return
	 */
	
	public static void setSettingFpReadingPageOffOn(boolean value) {
		if (value == false) {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_READING, 0);
		} else {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_READING, 1);
		}
	}
	
	public static boolean isSettingFpReadingPageOffOn() {
		int able = Settings.System.getInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_READING, 0);
		return able == 1?true:false;
	}
	
	/**
	 * 设置指纹解锁用于应用锁开关
	 * @param value
	 */
	public static void setSettingFpAppLockOffOn(boolean value) {
	    if (value == false) {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_APPLOCK, 0);
			Settings.System.putString(AppData.getContext().getContentResolver(), PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME,"");
			if (isSettingFpScreenLockOn() == false) {//如果两者同时关闭，则进入powerdown模式
		        //记得在这里进入powerdown模式
			}

	    } else {
	    	Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_APPLOCK, 1);
	    }
	}
	/**
	 * 指纹用于应用解锁是否打开
	 */
	public static boolean isSettingFpAppLockOn() {
		int able = Settings.System.getInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_APPLOCK, 0);
		return able == 1?true:false;
	}
	/**
	 * 当应用解锁界面显示到屏幕上时要设置true，应用解锁界面退出屏幕（不管是否解锁成功）后，要设置为false
	 */
	public static void setLockUIStatus(boolean lock) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(AppData.getContext());
		preferences.edit().putBoolean("lockuirunning", lock).commit();
	}
	/**
	 * 判断解锁界面是否显示在屏幕上
	 */
	public static boolean getLockUIStatus() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(AppData.getContext());
		return preferences.getBoolean("lockuirunning", false);
	}
	
	//davie
	public static void setSettingFpNavFuncOffOn(boolean value) {
	    if (value == false) {
			Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_NAVFUNC, 0);
	    } else {
	    	Settings.System.putInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_NAVFUNC, 1);
	    }
	}
	public static boolean isSettingFpNavFuncOn() {
		int able = NavCtrlJni.getSta();
		return able == 1?true:false;
		/*
		int able = Settings.System.getInt(AppData.getContext().getContentResolver(), SYSTEM_SETTINGS_FP_NAVFUNC, 0);
		return able == 1?true:false;
		*/
	}
	public static int gotoSetNavFunc(boolean enable){	
		int ret = 0;
		if(enable){
			ret = NavCtrlJni.init(null);
		}else{
			ret = NavCtrlJni.uinit();
		}
		
		return ret;
	}
}
