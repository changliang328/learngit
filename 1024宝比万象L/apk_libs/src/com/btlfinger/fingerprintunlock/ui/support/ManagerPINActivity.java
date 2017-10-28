package com.btlfinger.fingerprintunlock.ui.support;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import com.btlfinger.fingerprintunlock.support.ChooseLockSettingsHelper;
import com.android.internal.widget.LockPatternUtils;
import android.app.admin.DevicePolicyManager;
import android.util.Log;
import android.content.Context;
import com.android.internal.widget.LockPatternUtils;



public class ManagerPINActivity extends Activity {

    LockPatternUtils mPatternUtils;
	private final String TAG = "ManagerPINActivity";
	private int backupPasswordQuality =  DevicePolicyManager.PASSWORD_QUALITY_NUMERIC;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.mPatternUtils = new LockPatternUtils(this);
        if (this.mPatternUtils.getKeyguardStoredPasswordQuality() >= DevicePolicyManager.PASSWORD_QUALITY_SOMETHING) {
            new ChooseLockSettingsHelper(this, null)
            .launchConfirmationActivity(100, null, null);
            return;

        }
		//mPatternUtils.setLockPatternEnabled(false);
		//mPatternUtils.clearLock(false);
		Log.i(TAG, "getKeyguardStoredPasswordQuality" + mPatternUtils.getKeyguardStoredPasswordQuality());

		Intent localIntent = new Intent("android.app.action.SET_NEW_PASSWORD");
        localIntent.setPackage("com.android.settings");
        localIntent.putExtra("minimum_quality", DevicePolicyManager.PASSWORD_QUALITY_SOMETHING);
        //localIntent.putExtra("minimum_quality",DevicePolicyManager.PASSWORD_QUALITY_NUMERIC);
        startActivityForResult(localIntent, 110);
        //Intent intent = createIntent(this,DevicePolicyManager.PASSWORD_QUALITY_NUMERIC,false,4, 17, false,false);
        //startActivityForResult(intent,110);
    }

    public static Intent createIntent(Context context, int quality, final boolean isFallback,
                                      int minLength, final int maxLength, boolean requirePasswordToDecrypt,
                                      boolean confirmCredentials) {
        Intent intent = new Intent();
        intent.setClassName("com.android.settings", "com.android.settings.ChooseLockPassword");
        intent.putExtra(LockPatternUtils.PASSWORD_TYPE_KEY, quality);
        intent.putExtra("lockscreen.password_min", minLength);
        intent.putExtra("lockscreen.password_max", maxLength);
        intent.putExtra("confirm_credentials", confirmCredentials);
        intent.putExtra("extra_require_password", requirePasswordToDecrypt);
        return intent;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub

        super.onActivityResult(requestCode, resultCode, data);
        int quality = this.mPatternUtils.getKeyguardStoredPasswordQuality();
        Log.i(TAG, "requestCode" + requestCode + "resultCode" + resultCode + "quality" + quality);

        if (requestCode == 110 && resultCode != -1 && quality == DevicePolicyManager.PASSWORD_QUALITY_SOMETHING ) {
            finish();
            return;
        } else if (requestCode == 110 && resultCode != 1 && quality == DevicePolicyManager.PASSWORD_QUALITY_NUMERIC ) {
            finish();
            return;
        } else if (requestCode == 110 && resultCode != -1 && quality ==0 ) {
            finish();
            return;
        } else if (requestCode == 100 && resultCode != -1) {
            finish();
            return;
        }
        Intent intent = new Intent();
        intent.setClass(this, ManageFmActivity.class);
        startActivityForResult(intent, 0);
        finish();


    }
}
