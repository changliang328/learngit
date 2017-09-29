package com.btlfinger.fingerprintunlock.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.util.Log;

import com.btlfinger.fingerprintunlock.PackagesConstant;
/**
 * 应用锁广播接收
 * 
 * @author blestech
 * @since 2015-11-26
 */

public class AppLockReceiver extends BroadcastReceiver {

	private String INTENT_UPDATE_LAST_PACKAGE_NAME = "com.btlfinger.fingerprint.update.last_package_name";

	@Override
	public void onReceive(Context context, Intent intent) {
		if (intent.getAction().equals(INTENT_UPDATE_LAST_PACKAGE_NAME)) {
		    String packageName = intent.getStringExtra("packagename");
		    if (packageName != null) {
		        Settings.System.putString(context.getContentResolver(), PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME, packageName);
		    }
		}
	}
	
}
