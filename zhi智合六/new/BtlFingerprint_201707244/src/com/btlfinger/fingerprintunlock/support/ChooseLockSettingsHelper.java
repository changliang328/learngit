package com.btlfinger.fingerprintunlock.support;
import android.app.Activity;
import android.app.Fragment;
import android.content.Intent;
import com.android.internal.widget.LockPatternUtils;
import android.util.Log;
import android.app.admin.DevicePolicyManager;
import com.btlfinger.fingerprintunlock.applock.LockUI;


public class ChooseLockSettingsHelper {
	private Activity mActivity;
	private Fragment mFragment;
	private LockPatternUtils mLockPatternUtils;

	public ChooseLockSettingsHelper(Activity paramActivity) {
		this.mActivity = paramActivity;
		this.mLockPatternUtils = new LockPatternUtils(paramActivity);
	}

	public ChooseLockSettingsHelper(Activity paramActivity,
			Fragment paramFragment) {
		this(paramActivity);
		this.mFragment = paramFragment;
	}

	private boolean confirmPassword(int paramInt,
			CharSequence paramCharSequence, boolean paramBoolean) {
		Log.i("aaa", "confirmPassword: ");
		if (!this.mLockPatternUtils.isLockPasswordEnabled()) {
			//((LockUI) mActivity).onActivityResult(100,Activity.RESULT_OK,null);
			((LockUI) mActivity).showNoPwd();
			return false;
		}
		Intent localIntent = new Intent();
		localIntent.putExtra("com.android.settings.ConfirmLockPattern.header",
				paramCharSequence);
		localIntent.setClassName("com.android.settings",
				"com.android.settings.ConfirmLockPassword");
		if (this.mFragment != null) {
			this.mFragment.startActivityForResult(localIntent, paramInt);
		}

		this.mActivity.startActivityForResult(localIntent, paramInt);

		return true;
	}

	private boolean confirmPattern(int paramInt,
			CharSequence paramCharSequence1, CharSequence paramCharSequence2,
			boolean paramBoolean) {
		Log.i("aaa", "confirmPattern: ");
		if ((!this.mLockPatternUtils.isLockPatternEnabled())
				|| (!this.mLockPatternUtils.savedPatternExists())) {
			//((LockUI) mActivity).onActivityResult(100,Activity.RESULT_OK,null);
			((LockUI) mActivity).showNoPwd();
			return false;
		}
		Intent localIntent = new Intent();
		localIntent.setClassName("com.android.settings",
				"com.android.settings.ConfirmLockPattern");
		if (this.mFragment != null) {
			this.mFragment.startActivityForResult(localIntent, paramInt);
		}
		this.mActivity.startActivityForResult(localIntent, paramInt);

		return true;
	}

	public boolean launchConfirmationActivity(int paramInt,
			CharSequence paramCharSequence1, CharSequence paramCharSequence2) {
		Log.i("aaa", "launchConfirmationActivity: ");
		return launchConfirmationActivity(paramInt, paramCharSequence1,
				paramCharSequence2, false);
	}

	boolean launchConfirmationActivity(int paramInt,
			CharSequence paramCharSequence1, CharSequence paramCharSequence2,
			boolean paramBoolean) {
		Log.i("aaa", "launchConfirmationActivity: ");
		Log.i("aaa",
				"launchConfirmationActivity: "
						+ mLockPatternUtils.getKeyguardStoredPasswordQuality());
		switch (this.mLockPatternUtils.getKeyguardStoredPasswordQuality()) {

		case DevicePolicyManager.PASSWORD_QUALITY_SOMETHING:
			return confirmPattern(paramInt, paramCharSequence1,
					paramCharSequence2, paramBoolean);
		case DevicePolicyManager.PASSWORD_QUALITY_NUMERIC:
			return confirmPassword(paramInt, paramCharSequence1, paramBoolean);
		default:
			return confirmPassword(paramInt, paramCharSequence1, paramBoolean);
		}
	}
}
