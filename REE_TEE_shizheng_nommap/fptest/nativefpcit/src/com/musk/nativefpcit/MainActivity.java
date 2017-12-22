package com.musk.nativefpcit;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.Vibrator;
import android.provider.Settings;
import android.text.StaticLayout;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.provider.Settings;
import android.widget.Toast;
import android.content.*;

import com.android.internal.widget.LockPatternUtils;
import com.android.internal.widget.LockPatternUtils.RequestThrottledException;

import android.os.UserHandle;
import android.app.admin.DevicePolicyManager;
import android.hardware.fingerprint.Fingerprint;
import android.hardware.fingerprint.FingerprintManager;
import android.hardware.fingerprint.FingerprintManager.AuthenticationCallback;
import android.hardware.fingerprint.FingerprintManager.AuthenticationResult;
import android.hardware.fingerprint.FingerprintManager.RemovalCallback;
import android.widget.Toast;
import android.os.CancellationSignal;
import android.view.Menu;
import android.view.MenuItem;
import java.util.List;

public class MainActivity extends Activity {
	private static final String TAG = "citfingerprint";

	private static final int VERSION_ANDROID_N = 0;


	public static final String FINGER_RESULT = "finger_result";
	private static final int REQUEST_CODE = 1;
	private static final int RESULT_CODE = 1;

	private static final int TEST_FAIL = 0;
	private static final int TEST_PASS = 1;

	private boolean mIsTestOk = false;

	private Button mTestButton = null;
	// add by CIT SYSTEM
	private Button btn_passBtn = null;
	private Button btn_failBtn = null;
	private Button btn_reTestBtn = null;

	private Vibrator vib_vibrator = null;

	private TextView mResultTV = null;

	private static int TEST_ONE = 0;

	private static boolean mhasClickIc = false;
	
	private static final int UPDATE_RESULT = 1;
	private static final int FINGERPRINT_TEMP_EXIST = 1001;
	private static final int FINGERPRINT_ERROR_CANCELED = 5;
	
	
	private static final int START_INIT_DATA = 2 ; 
	
	private static final int START_TO_ENROLL = 1 ; 

	private Intent mData = null;
	private int mEnrollmentSteps = -1;
	private int mEnrollmentRemaining = -1;
	private CancellationSignal mEnrollmentCancel = null;
	private boolean mEnrolling;
	protected LockPatternUtils mUtils;
	private int mRequestedQuality = DevicePolicyManager.PASSWORD_QUALITY_ALPHABETIC;
	private FingerprintManager mFingerprintManager;
	private int userId = 0;
	private byte[] mToken = null;
	static long[] pattern = { 200, 50 };
	private boolean hascancell = false;
	private boolean ispause = false;
	private HandlerThread mHandlerThread = null;
	private Handler handler = null ;
	
	private Handler mUIHandler = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
    ispause=false;
		if (0 != init()) {
			Log.e(TAG, "init failed ");
		}
		
		

	}

	@Override
	protected void onStart() {
		Log.i(TAG, "onStart()");
		//TEST_ONE = 1;		
		super.onStart();
	}

	@Override
	protected void onResume() {

		Log.i(TAG, "onResume 0000");
		mhasClickIc=false;
		ispause=false;
		TEST_ONE = 1;	
		mResultTV.setText(R.string.test_init);
		//handler.removeMessages(START_INIT_DATA);
		handler.sendEmptyMessage(START_INIT_DATA);
		super.onResume();
	}

	@Override
	protected void onPause() {
		Log.i(TAG, "onPause 11111");
    ispause=true; 
		setResultAndClean();

		super.onPause();
	}

	@Override
	protected void onStop() {
		Log.i(TAG, "onStop11111");
		handler.removeMessages(START_INIT_DATA);
		btn_passBtn.setEnabled(false);
		btn_failBtn.setEnabled(true) ;
		btn_passBtn.setTextColor(Color.rgb(0, 255, 0));
		btn_failBtn.setTextColor(Color.rgb(255, 0, 0));
		mResultTV.setText(R.string.tip_press_ic);
		super.onStop();
	}

	@Override
	public void onDestroy() {
		vib_vibrator.cancel();
		//mHandlerThread.interrupt();
		mHandlerThread = null;
		Log.d(TAG, "onDestroy ret = ");
		super.onDestroy();
	}

	@Override
	public void onRequestPermissionsResult(int requestCode,
			String[] permissions, int[] grantResults) {
		switch (requestCode) {
		case REQUEST_CODE:
			if (grantResults.length > 0
					&& grantResults[0] == PackageManager.PERMISSION_GRANTED) {
				Log.i(TAG, "has permission");
				startEnrollment();
			} else {
				Log.e(TAG, "no permission");
			}

			break;

		default:
			break;
		}

	}

	private class TestButtonClickListener implements OnClickListener {

		@Override
		public void onClick(View view) {
			if (null == view)
				return;

			switch (view.getId()) {
			case R.id.btn_retest:
				TEST_ONE = 1;
				Log.i(TAG, "click btn_retest 11111retest  ");
				Log.d(TAG, "mhasClickIc= "+mhasClickIc);
				if(mhasClickIc){
				mhasClickIc=false;
				btn_passBtn.setEnabled(false);
				btn_failBtn.setEnabled(true) ;		
				startEnrollment();
				}
				hascancell = false;
			  
				break;

			case R.id.btn_pass:
		
				mData.putExtra(FINGER_RESULT, TEST_PASS);
				Log.i(TAG, "click btn_pass 2222  set result pass  ");
        //setResultAndClean();
         setActivityResult(RESULT_CODE, mData);
				finish();
				break;
			case R.id.btn_fail:
	
				mData.putExtra(FINGER_RESULT, TEST_FAIL);
				Log.i(TAG, "click btn_fail 3333  set result fail  ");
       // setResultAndClean();
        setActivityResult(RESULT_CODE, mData);
				finish();
				break;
			default:
				break;

			}

		}

	}

	private void initEnrollData() {
		final String mChosenPassword = "1234";
		userId = UserHandle.myUserId();
		Log.i(TAG, "userId = " + userId);
		mUtils = new LockPatternUtils(this);

		Log.d(TAG,
				"mUtils.getActivePasswordQuality(userId) quality="
						+ mUtils.getActivePasswordQuality(userId));

		if (mUtils.getActivePasswordQuality(userId) != DevicePolicyManager.PASSWORD_QUALITY_UNSPECIFIED) {

			Log.d(TAG,
					"alreay set screen lock ,now remove screenLock when first in Fptest!");
			removeScreenLock();

		}
		final long challenge = mFingerprintManager.preEnroll();
		Log.d(TAG, "mChosenPassword=" + mChosenPassword + "----mChallenge="
				+ challenge + "----userId=" + userId);
		try {
			//fixbug: 姝よ缃瘑鐮佸嚱鏁颁細鏈夎緝闀垮欢鏃讹紝鍥犲钩鍙颁笉鍚屼細鏈�-5s鏃堕棿娑堣�銆�					
			startSavePassword(false, challenge, mChosenPassword, null,
					mRequestedQuality);
			Log.d(TAG, "ispause=" +ispause);
      if(!ispause){
			 mToken = mUtils.verifyPassword(mChosenPassword, challenge,
					userId);
			}else{
			 return ;
			}			
			handler.sendEmptyMessage(START_TO_ENROLL);
		} catch (RequestThrottledException e) {
			mToken = null;
			Log.i(TAG, "token  RequestThrottledException");
		}
      
	}

	private int startEnrollment() {
		if (checkSelfPermission(Manifest.permission.MANAGE_FINGERPRINT) != PackageManager.PERMISSION_GRANTED) {
			Log.i(TAG, " check permission");

			if (shouldShowRequestPermissionRationale(Manifest.permission.MANAGE_FINGERPRINT)) {

				Toast.makeText(MainActivity.this,
						"no permnission ,Please get permission",
						Toast.LENGTH_SHORT).show();
			} else {
				requestPermissions(
						new String[] { Manifest.permission.MANAGE_FINGERPRINT },
						REQUEST_CODE);
			}

		} else {
			//mUIHandler.sendEmptyMessageDelayed(1001, 500);
			mUIHandler.sendEmptyMessage(1001);
			
		}

		return 0;

	}

	private int doEnrollTest() {

		Log.e(TAG, "doEnrollTest() ");
		// zhangyin  remove in androidM 
		if (userId != UserHandle.USER_NULL) {
			mFingerprintManager.setActiveUser(userId);
		}
		mResultTV.setText(R.string.tip_press_ic);
		mEnrollmentCancel = new CancellationSignal();
		Log.e(TAG, "doEnrollTest() mEnrollmentCancel = new CancellationSignal();");
		try {
			if (null == mToken) {
				Log.i(TAG, "#####token is null ,please try again");
				
				Toast.makeText(this, R.string.retest_tips, Toast.LENGTH_SHORT).show() ; 
				return -1;
			}
			// android N need a paramter // zhangyin锛歳emove userId in androidM
			mFingerprintManager.enroll(mToken, mEnrollmentCancel, 0,userId,
					mEnrollmentCallback);
			/*mFingerprintManager.enroll(mToken, mEnrollmentCancel, 0,
					mEnrollmentCallback);	*/	
    Log.e(TAG, "doEnrollTest() enroll;");
		} catch (Exception e) {
			Log.e("fptest", "start enrollfail" + e.toString());
			mEnrolling = false;
		}
		mEnrolling = true;

		return 0;
	}

	private int init() {

		btn_passBtn = (Button) findViewById(R.id.btn_pass);
		btn_failBtn = (Button) findViewById(R.id.btn_fail);
		btn_reTestBtn = (Button) findViewById(R.id.btn_retest);

		mResultTV = (TextView) findViewById(R.id.test_result);
		vib_vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
		
		mResultTV.setTextColor(Color.rgb(0, 255, 0));
		btn_passBtn.setTextColor(Color.rgb(0, 255, 0));
		btn_failBtn.setTextColor(Color.rgb(255, 0, 0));
		mData = new Intent();

		if (null == mResultTV) {
			Log.e(TAG, "init view failed");
			return -1;
		}
		
		btn_passBtn.setEnabled(false);
		btn_failBtn.setEnabled(true) ; 

		TestButtonClickListener clickListner = new TestButtonClickListener();
		btn_passBtn.setOnClickListener(clickListner);
		btn_failBtn.setOnClickListener(clickListner);
		btn_reTestBtn.setOnClickListener(clickListner);
		mFingerprintManager = getSystemService(FingerprintManager.class);
		mHandlerThread = new HandlerThread("CSFinger_CIT");
		mHandlerThread.start();
		
		handler = new Handler(mHandlerThread.getLooper()) {
			@Override
			public void handleMessage(Message msg) {
				if (msg.what == START_TO_ENROLL) {					
					startEnrollment();
				}else if (msg.what == START_INIT_DATA) {
					initEnrollData();
				}
			}
		};
		
		mUIHandler = new Handler(getMainLooper()){
			@Override
			public void handleMessage(Message msg) {
				if (msg.what == 1001) {
					doEnrollTest();
				}
			}
		};
		return 0;
	}

	private void removeScreenLock() {
		Log.d(TAG, "start remove screen lock!");
		mUtils.clearLock(userId);
		Log.d(TAG, "clearLock! userid" + userId);
		mUtils.setLockScreenDisabled(false, userId);
		Log.d(TAG, "setLockScreenDisabled!");
		mUtils.setCredentialRequiredToDecrypt(false);

		Log.d(TAG, "setCredentialRequiredToDecrypt  !");
		mUtils.clearEncryptionPassword();
		Log.d(TAG, "clearEncryptionPassword  !");
		Log.d(TAG, "removeScreenLock success!");
	}

	private void startSavePassword(boolean credentialRequired, long challenge,
			String chosenPassword, String currentPassword, int requestedQuality) {

		mUtils.setCredentialRequiredToDecrypt(credentialRequired);
		Log.i(TAG, "begin to saveLockPassword  " + userId);
		
		mUtils.saveLockPassword(chosenPassword, currentPassword,
				requestedQuality, userId);
		Log.i(TAG, "saveLockPassword finished  ");
	}

	/*
	 * protected void prepare(LockPatternUtils utils, boolean
	 * credentialRequired, boolean hasChallenge, long challenge) {
	 * //mUtils.isSecure(UserHandle.myUserId());
	 * mUtils.setCredentialRequiredToDecrypt(credentialRequired); }
	 */

	private void delays(int time) {
		try {
			Thread.sleep(time);
		} catch (Exception e) {

		}
	}

	public FingerprintManager.EnrollmentCallback mEnrollmentCallback = new FingerprintManager.EnrollmentCallback() {

		@Override
		public void onEnrollmentProgress(int remaining) {
			Log.d(TAG, "onEnrollmentProgress remaining=" + remaining);
      mhasClickIc=true;
			if (mEnrollmentSteps == -1) {
				mEnrollmentSteps = remaining;
			}
			cancelEnrollment();
			hascancell = true;
			mEnrollmentRemaining = remaining;
			onEnrollmentProgressChange(mEnrollmentSteps, remaining);
			if (remaining == 0) {
				// complete
				// onEnrollmentComplete();
				Log.e(TAG, "enroll completed ....");
			}

		}

		@Override
		public void onEnrollmentHelp(int helpMsgId, CharSequence helpString) {
			Log.e(TAG, "CustomFingerManager onEnrollmentHelp");
       mhasClickIc=true;
			// onEnrollmentHelp(helpString);
			Log.e(TAG, "onEnrollmentHelp======" + helpString);
			// mTestButton.setBackgroundColor(Color.GRAY);
			if (TEST_ONE == UPDATE_RESULT) {
				mIsTestOk = true;
				mResultTV.setText(R.string.pass_test);
				mResultTV.setTextColor(Color.rgb(0, 255, 0));

				btn_passBtn.setEnabled(true);
				btn_failBtn.setEnabled(false) ; 

				// mTestButton.setClickable(true);
				cancelEnrollment();
				hascancell = true;


			}

			mData.putExtra(FINGER_RESULT, TEST_PASS);

			TEST_ONE--;

			setActivityResult(RESULT_CODE, mData);

		}

		@Override
		public void onEnrollmentError(int errMsgId, CharSequence errString) {
			Log.e(TAG, "onEnrollmentError=" + errMsgId + "  errString="
					+ errString);
      mhasClickIc=true;
			cancelEnrollment();
			hascancell = true;

			if (errMsgId == FINGERPRINT_TEMP_EXIST) {
				mIsTestOk = true;
				// mTestButton.setBackgroundColor(Color.GRAY);
				mResultTV.setText(R.string.pass_test);
				mResultTV.setTextColor(Color.rgb(0, 255, 0));

				// mTestButton.setClickable(true);

				btn_passBtn.setEnabled(true);
				btn_failBtn.setEnabled(false);
				//Settings.Global.putInt(getContentResolver(),
					//	FINGER_TEST_RESULT, TEST_PASS);

			} else if (errMsgId == FINGERPRINT_ERROR_CANCELED) {
				Log.e(TAG, "onEnrollmentError cancelEnrollment");

			} else {
				mIsTestOk = false;
				// mTestButton.setBackgroundColor(Color.GRAY);
				mResultTV.setText(R.string.fail_test);
				mResultTV.setTextColor(Color.rgb(255, 0, 0));

				// mTestButton.setClickable(true);
				btn_passBtn.setEnabled(false);
				btn_failBtn.setEnabled(true);



			}

      mData.putExtra(FINGER_RESULT, TEST_PASS);
			TEST_ONE--;

		}
	};

	private RemovalCallback mRemoveCallback = new RemovalCallback() {

		@Override
		public void onRemovalSucceeded(Fingerprint fingerprint) {
			Log.e(TAG, "onRemovalSucceeded");
		}

		@Override
		public void onRemovalError(Fingerprint fp, int errMsgId,
				CharSequence errString) {
			Log.e(TAG, "onRemovalError");
		}
	};

	public void onEnrollmentProgressChange(int steps, int remaining) {

		Log.i(TAG, "onEnrollmentProgressChange step = " + steps + "remaining ="
				+ remaining);
	  Log.i(TAG, "onEnrollmentProgressChange TEST_ONE = " + TEST_ONE );
		// mTestButton.setBackgroundColor(Color.GRAY);
		if (remaining > 0 && TEST_ONE == UPDATE_RESULT) {
			Log.i(TAG, "onEnrollmentProgressChange pass_test = ");
			mIsTestOk = true;
			mResultTV.setText(R.string.pass_test);
			mResultTV.setTextColor(Color.rgb(0, 255, 0));
			// mTestButton.setClickable(true);
			vib_vibrator.vibrate(pattern, -1);

			btn_passBtn.setEnabled(true);
			btn_failBtn.setEnabled(false);


			mData.putExtra(FINGER_RESULT, TEST_PASS);

		}

		TEST_ONE--;

	}

	/**
	 * // it can't remove all fingerprint templates below android Nougat
	 */
	private void removeAllFingerprintTemplatesAndFinish() {
		final List<Fingerprint> items = mFingerprintManager.getEnrolledFingerprints();
		if (mFingerprintManager != null
				&& mFingerprintManager.isHardwareDetected()
				&& mFingerprintManager.getEnrolledFingerprints().size() > 0) {
			if (items.size() > 0) {  
	        Log.d(TAG, "Fingerprint = " + items.size());  
            for (Fingerprint i : items) {  
                mFingerprintManager.remove(i,userId, mRemoveCallback);  
            } 
        }  
			/* mFingerprintManager.remove(new Fingerprint(null, 0, 0,
			 0),userId,mRemoveCallback);*/
		} else {
			Log.d(TAG, "removeAllFingerprintTemplatesAndFinish | 2");
		}
	}

	public void setActivityResult(int requestCode, Intent data) {
		setResult(requestCode, data);
	}

	public void cancelEnrollment() {
		if (mEnrolling) {
			mEnrollmentCancel.cancel();
			mEnrolling = false;
			mEnrollmentSteps = -1;
			Log.i(TAG, "cancel sucess11");
		}
	}

	@Override
	public void onBackPressed() {
		Log.e(TAG, "onBackPressed");
		return;

	}

	private void setResultAndClean() {
		//if (hascancell) {
			cancelEnrollment();
		//}
		removeScreenLock();
		removeAllFingerprintTemplatesAndFinish();
		setActivityResult(RESULT_CODE, mData);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.main, menu);

		return true;
	}
}
