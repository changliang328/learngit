package com.btlfinger.fingerprint.utils;
import android.content.Context;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
/**
 * App信息工具类
 * 
 * @author blestech
 * @since 2015-11-26
 */

public class AppInfoUtils {
	/**
	 * 获取应用程序的版本名称
	 * @param context 上下文
	 * @return 版本名称
	 */
	public static String getAppVersionName(Context context){
		try {
			PackageManager pm = context.getPackageManager();
			PackageInfo  packinfo = pm.getPackageInfo(context.getPackageName(), 0);
			String versionName = packinfo.versionName;
			return versionName;
		} catch (NameNotFoundException e) {
			e.printStackTrace();
			//根本不可能到达
			return "";
		}
	}
}
