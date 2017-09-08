package com.btlfinger.aidlservice;

import com.btlfinger.fingerprint.FingerPrintManager;
import com.btlfinger.fingerprint.FingerPrintModel;
import com.btlfinger.fingerprint.FpNative;
import com.btlfinger.fingerprint.dao.DbHelper;
import com.btlfinger.fingerprint.dao.FpsTable;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
import com.btlfinger.service.aidl.IFpsFingerClient;
import com.btlfinger.service.aidl.IFpsFingerManager;
import com.btlfinger.fingerprintunlock.PackagesConstant;
import com.btlfinger.fingerprintunlock.R;

import android.app.KeyguardManager;
import android.app.Service;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.net.Uri;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.Vibrator;
import android.telephony.TelephonyManager;
import android.telephony.PhoneStateListener;
import android.util.Log;
import android.provider.Settings;


import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import com.android.internal.policy.IKeyguardService;
import android.app.ActivityManager;
import android.view.KeyEvent;
import android.hardware.input.InputManager;
import android.os.SystemClock;
import android.content.pm.PackageManager;
/**
 * 指纹服务操作
 *
 * @author blestech
 * @since 2015-11-26
 */
public class FingerService extends Service {

    public static String TAG = "FingerService";

    public static final boolean DEBUG = true;
    public static final String KEYGUARD_PACKAGE = "com.android.systemui";
    public static final String KEYGUARD_CLASS = "com.android.systemui.keyguard.KeyguardService";
    public static final String FINGER_PRINT_UNMATCH_ACTION = "com.blestech.finger.action.UNMATCH";
    public static final String FINGER_PRINT_MATCH_ACTION = "com.blestech.fingerprint.match";
    public static final String INTENT_ACTION_ORBIT_FLEX = "android.intent.action.ORBIT_FLEX";
    public static final String FINGER_PRINT_PUT_KEYCODE = "android.blestech.action.put_keycode";
    public static final String FINGER_PRINT_PUT_INTERRUPTCODE = "android.blestech.action.put_interruptcode";
	public static final String BOOT_COMPLETED = "android.intent.action.BOOT_COMPLETED";

    private KeyguardManager.KeyguardLock mLock;
    private KeyguardManager mKeyguardManager;
    private TelephonyManager mTelemanager;
    private PowerManager mPowerManager;
    private PowerManager.WakeLock mWakeLock;
    private PowerManager.WakeLock mLightLock;
    private IKeyguardService mKeyguardService;
    private boolean mUserPresent = false;
    private Vibrator mVib;
    private int mMatchType ; //0: nomatch ; 1:black screen  match ; 2: screen on  match
    public static final int INIT_REGISTER_WHAT = 1;
    public static final int REGISTER_WHAT = 2;
    public static final int MATCH_AGAIN = 3;
    public static final int SHUTDOWN_WHAT = 4;
    public static final int POWERDOWN_WHAT = 5;
    public static final int ISFINGERUP_WHAT = 7;
    public static final int MATCH_FAIL = 9;
    public static final int MATCH_WHAT = 8;
    public static final int MATCH_SUCCESS = 10;
    public static final int MATCH_CHECKUPS = 15;
    public static final int MATCH_CHECKUPE = 16;
	public static final int MII_TEST = 19;
	public static final int ISFINGERUP = 20;
    public static final int PUT_KEYCODE = 11;
	public static final int MAX_CHECK_FINGERUP = 20;
    public static final int FINGER_FUCTION= 21;

    public int mFingerprintUp;
    public int mMatchErrorCounts = 0;
	public int fingerup_counts = 0;
    private Context mContext;

    private FpNative mFpNative = null;
    private IFpsFingerClient mClient;
	private boolean isRing = false;
	private boolean mtest = false;
    private final String SYS_PATH = "/sys/bl229x_sysfs/keycode";

    @Override
    public IBinder onBind(Intent arg0) {
        return mBinder;
    }

    private FingerPrintManager fingerDataManager = null;
	//private static String DEFAULT_FP_NAME = "Fingerprint";

    private HandlerThread mFingerThread = new HandlerThread("FingerThread");
    private Handler mHandler = null;
    private DbHelper mDbHelper = null;

    private final IFpsFingerManager.Stub mBinder = new IFpsFingerManager.Stub() {

        @Override
        public String getValue(String name) throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "getValue");

            return null;
        }

        @Override
        public int update(String name, String value, int attribute) throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "update");

            return 0;
        }

        @Override
        public void waitScreenOn() throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "waitScreenOn");
            mHandler.sendEmptyMessage(SHUTDOWN_WHAT);
            //mHandler.removeMessages(MATCH_AGAIN);
        }

        @Override
        public void powerDown() throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "powerDown");
            mHandler.sendEmptyMessage(POWERDOWN_WHAT);
            //mHandler.removeMessages(MATCH_AGAIN);
        }

        @Override
        public void initRegister() throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "initRegister");
            mHandler.sendEmptyMessage(INIT_REGISTER_WHAT);
        }

        @Override
        public void register() throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "register");
            mHandler.sendEmptyMessage(REGISTER_WHAT);
        }

        @Override
        public void saveData() throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "saveData");
            int fingerMaxIndex = fingerDataManager.getMaxId() + 1;
            String tempName = FingerPrintManager.FP_DATA_DIR + fingerMaxIndex;
            int templateSize = mFpNative.FpSaveTemplate(fingerMaxIndex);
            int mFpNameIndex = fingerDataManager.getNewFpNameIndex();

            FingerPrintModel parcelmodel = new FingerPrintModel();

            parcelmodel.fp_data_index = fingerMaxIndex;
            parcelmodel.fp_template_size = templateSize;
			//parcelmodel.fp_name = DEFAULT_FP_NAME + mFpNameIndex;
			parcelmodel.fp_name = getString(R.string.fingerprint) + mFpNameIndex;
            parcelmodel.fp_data_path = fingerMaxIndex;

            Log.d(TAG,"name:"+parcelmodel.fp_name+ "fingerMaxIndex:"+ fingerMaxIndex);

            mDbHelper.insertNewFpData(parcelmodel);
        }

        @Override
        public void saveDataWithName(String name) throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "saveDataWithName");

            Log.d(TAG,"fingerMaxIndex: "+ fingerDataManager.getMaxId());
            int fingerMaxIndex = fingerDataManager.getMaxId() + 1;
            String tempName = FingerPrintManager.FP_DATA_DIR + fingerMaxIndex;
            int templateSize = mFpNative.FpSaveTemplate(fingerMaxIndex);
            int mFpNameIndex = fingerDataManager.getNewFpNameIndex();

            FingerPrintModel parcelmodel = new FingerPrintModel();

            parcelmodel.fp_data_index = fingerMaxIndex;
            parcelmodel.fp_template_size = templateSize;
            parcelmodel.fp_name = name;
            parcelmodel.fp_data_path = fingerMaxIndex;

            Log.d(TAG,"name:"+name+ "fingerMaxIndex:"+ fingerMaxIndex);

            mDbHelper.insertNewFpData(parcelmodel);
        }

        @Override
        public void isFingerUp() throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "isFingerUp");
            mHandler.sendEmptyMessage(ISFINGERUP_WHAT);
        }

        @Override
        public void matchUp(int nType) throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "matchUp");
				
				
			
            Message msg = mHandler.obtainMessage();
            msg.what = MATCH_AGAIN;
            msg.arg1 = nType;
            mHandler.sendMessage(msg);
        }

        @Override
        public void listen(IFpsFingerClient client) throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "listen");
            mClient = client;
        }

        @Override
        public void cancle(final int isRegis) throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "cancle");
            new Thread(new Runnable() {

                @Override
                public void run() {
                    mFpNative.FpCancelAction(isRegis);
                    if (DEBUG)
                        Log.e(TAG, "---cancle---running");
                }
            }).start();

            //mHandler.removeMessages(MATCH_AGAIN);
			writeGeneralKeycode();
        }

		@Override
		public int mmiFpTest() throws RemoteException {
			if (DEBUG)
				Log.i(TAG, "mmiFpTest");

			int ret = 0;
			//int ret = mFpNative.FpMmiFpTest();
			mtest = true;
			mFpNative.FpWriteKeycode(68);
			mFpNative.FpWaitScreenOn();
	
			return ret;
		}
    };

    @Override
    public void onCreate() {

        super.onCreate();
        if (DEBUG)
            Log.i(TAG, "onCreate");

        mMatchType = 0;

        mMatchErrorCounts = 0;

        mDbHelper = new DbHelper(this);

        fingerDataManager = new FingerPrintManager(this);

        mFpNative = new FpNative();

        mFingerThread.start();

        mHandler = new Handler(mFingerThread.getLooper(), callback);
        mKeyguardManager = (KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE);
        mLock = mKeyguardManager.newKeyguardLock("KeyguardLock");

        IntentFilter filter = new IntentFilter();

        // listen screen_on/off event
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);

        filter.addAction(Intent.ACTION_USER_PRESENT);
        filter.addAction(INTENT_ACTION_ORBIT_FLEX);
        filter.addAction(FINGER_PRINT_MATCH_ACTION);
		filter.addAction(BOOT_COMPLETED);

		filter.setPriority(Integer.MAX_VALUE);
        registerReceiver(mScreenReceiver, filter);

        mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mVib = (Vibrator) getSystemService(Service.VIBRATOR_SERVICE);

        mWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
                                              this.getClass().getCanonicalName());
        mLightLock = mPowerManager.newWakeLock(PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
                                               this.getClass().getCanonicalName());
		
		mUserPresent = !mKeyguardManager.isKeyguardLocked();
        TelephonyManager tm = (TelephonyManager)getSystemService(Service.TELEPHONY_SERVICE);

        tm.listen(phonestatelistener, PhoneStateListener.LISTEN_CALL_STATE);

        bindService(this);
		
		Settings.System.putInt(getContentResolver(), "com_btlfinger_fingerprint_up_flag", 0);
    }

    PhoneStateListener phonestatelistener = new PhoneStateListener() {

        @Override
        public void onCallStateChanged(int state, String incomingNumber) {

            super.onCallStateChanged(state, incomingNumber);
            Message msg = null;
            switch (state) {

            case TelephonyManager.CALL_STATE_IDLE:
                if (DEBUG)
                    Log.i(TAG, "CALL_STATE_IDLE");
				isRing = false;
				writeGeneralKeycode();
                break;
            case TelephonyManager.CALL_STATE_OFFHOOK:
                if (DEBUG)
                    Log.i(TAG, "CALL_STATE_OFFHOOK");
                clearCallKeycode();
                break;
            case TelephonyManager.CALL_STATE_RINGING:
                if (DEBUG)
                    Log.i(TAG, "CALL_STATE_RINGING");
				Settings.System.putString(getContentResolver(), "com_btlfinger_fingerprint_lastapp_package",
				"com.android.dialer");
                if(PreferenceUtils.isSettingFpCallLockOn()) {
					isRing = true;
					try {
						mBinder.cancle(1);
					} catch (RemoteException e1) {
						e1.printStackTrace();
					}

                    writeCallKeycode();
                }
                break;
            default:
                // clearCallKeycode();
                break;
            }
        }
    };

    boolean playing = false;
    BroadcastReceiver mScreenReceiver = new BroadcastReceiver() {
        @SuppressWarnings("deprecation")
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            String action = intent.getAction();
            boolean isScreenOn = mPowerManager.isScreenOn();
            boolean isKeygurad = mKeyguardManager.inKeyguardRestrictedInputMode();
            boolean isFingerLock = PreferenceUtils.isSettingFpScreenLockOn();
            if (DEBUG)
                Log.d(TAG, "onReceive action = " + action);
            if (DEBUG)
                Log.d(TAG, "isScreenOn = " + isScreenOn);
            if (DEBUG)
                Log.d(TAG, "isKeygurad = " + isKeygurad);

			if (action.equals(FINGER_PRINT_MATCH_ACTION)) {
                if (DEBUG)
                    Log.d(TAG, "onReceive FINGER_PRINT_MATCH_ACTION");
				if(playing){
					Log.d(TAG, "onReceive FINGER_PRINT_MATCH_ACTION exit");
					return;
				}
				playing = true;

				if(mtest && !isKeygurad){
					mtest = false;
					mHandler.sendEmptyMessage(MII_TEST);
					return;
				}

				if(isFingerLock){
					if(mUserPresent && !isKeygurad){						
						//mHandler.sendEmptyMessage(ISFINGERUP);			
					}else{
						//mVib.vibrate(100);						
						mFpNative.FpWriteKeycode(68);					
						if(mPowerManager.isScreenOn()){
							mMatchType = 2;
							//releaseWakeLock();
							mHandler.sendEmptyMessage(MATCH_WHAT); //start to match fingerprint
						}
						else if(isKeygurad){
							acquireWakeLock();
						    mMatchType = 1;
						    mHandler.sendEmptyMessage(MATCH_WHAT);
						}		
					}
				}
				mContext = context;
				mHandler.sendEmptyMessage(FINGER_FUCTION);
				
					
            } else if (action.equals(Intent.ACTION_SCREEN_ON) && isFingerLock) {
                if (DEBUG)
                    Log.d(TAG, "onReceive ACTION_SCREEN_ON");

				mMatchErrorCounts = 0;
                /*if((!mUserPresent || isKeygurad) && !isRing) {
					mFpNative.FpWriteKeycode(68);
					mHandler.sendEmptyMessage(SHUTDOWN_WHAT);
                }*/
            } else if (action.equals(Intent.ACTION_SCREEN_OFF) && isFingerLock) {
                if (DEBUG)
                    Log.d(TAG, "onReceive ACTION_SCREEN_OFF");

                mUserPresent = false;
                mLock.reenableKeyguard();
                mHandler.sendEmptyMessage(SHUTDOWN_WHAT);
		        Settings.System.putInt(getContentResolver(), "com_btlfinger_fingerprint_unlock_flag", 0);
                Settings.System.putString(getContentResolver(), "com_btlfinger_fingerprint_lastapp_package", "com.btlfinger.fingerprintunlock");

            } else if (action.equals(Intent.ACTION_USER_PRESENT) && mPowerManager.isScreenOn()) {
                if (DEBUG)
                    Log.d(TAG, "onReceive Intent.ACTION_USER_PRESENT ");
                mUserPresent = true;
				mHandler.sendEmptyMessage(SHUTDOWN_WHAT);
				//mFpNative.FpWriteKeycode(87);
                //mFpNative.FpWaitScreenOn();		
            } else if (action.equals(INTENT_ACTION_ORBIT_FLEX)) {
                if (DEBUG)
                    Log.d(TAG, "onReceive Intent.INTENT_ACTION_ORBIT_FLEX");
			} else if (action.equals(BOOT_COMPLETED) && isFingerLock) {
				mMatchErrorCounts = 0;
				mMatchType = 2;
				//mFpNative.FpWriteKeycode(68);
                mFpNative.FpWaitScreenOn();
            } else {
                if (DEBUG)
                    Log.d(TAG, "onReceive ERROR_NOTIFY_DIALOG_FINISH_ACTION times out of max mErrorCount = ");
            }
        }
    };

    private Callback callback = new Callback() {
        String dir;
        int[] result;
        int[] fileindex;
        int[] tempLength;
        int fileLength;
        int score = -1;
        @Override
        public boolean handleMessage(Message msg) {

            switch (msg.what) {
            case INIT_REGISTER_WHAT:
                if (DEBUG)
                    Log.d(TAG, "INIT_REGISTER_WHAT");
                mFpNative.FpInitRegister();
                break;

            case REGISTER_WHAT:
                if (DEBUG)
                    Log.d(TAG, "REGISTER_WHAT");
		    	try {
		    		if(isRing == true){
                    				mFpNative.FpWriteKeycode(169); 
						if (mClient != null)
							mClient.getValue(REGISTER_WHAT, -1);
						break;
		    		}
		 		} catch (RemoteException e1) {
                	e1.printStackTrace();
            	}

                score = mFpNative.FpRecordFingerprint();
                try {
                    if (mClient != null) {
                        mClient.getValue(REGISTER_WHAT, score);
						score = -1;
                    } else {
                        if (DEBUG)
                            Log.e(TAG, "register---is null---");
                    }
                } catch (RemoteException e1) {
                    e1.printStackTrace();
                }
                break;

            case MATCH_WHAT:
                if (DEBUG)
                    Log.d(TAG, "MATCH_WHAT mMatchType: " + mMatchType);
                //mHandler.removeMessages(MATCH_WHAT);
                //mHandler.removeMessages(MATCH_AGAIN);

                dir = fingerDataManager.FP_DATA_DIR;
                fileindex = fingerDataManager.getFileIndex();
                tempLength = fingerDataManager.getAllTemplateLength();
                fileLength = fileindex.length;

                result = mFpNative.FpMatchFingerprint(dir, fileindex, tempLength, fileLength);

				if(isRing){
					waitFinger();
					break;
				}
                if (DEBUG)
                    Log.d(TAG, "matchUp-result:" + result[0] + "--finger:" + result[1]);
                score = result[0];

                if (score > 0) {
					mVib.vibrate(100);
                    Message msg1 = mHandler.obtainMessage();
                    msg1.arg1 = result[1];
                    msg1.what = MATCH_SUCCESS;
                    mHandler.sendMessage(msg1);
                } else if (score != -2047 && mUserPresent != true) {
                    if (mMatchType == 0){
						waitFinger();
						break;
					}
                    if (mMatchType == 1) {
                        if ((score==-1) || (score==-100)){
							mVib.vibrate(100);
                        mHandler.sendEmptyMessage(MATCH_FAIL);
						}else{
							waitFinger();
						}               
                    } else if (mMatchType == 2) {
                    	if(!mPowerManager.isScreenOn()){
							waitFinger();
								break;
						}
                        if ((score==-1) || (score==-100)) {
							mVib.vibrate(100);
                            mMatchErrorCounts++;
							dispatchMatchInfo(5,mMatchErrorCounts,0);
							//mVib.vibrate(100);
							
                            /*if (mMatchErrorCounts < 2) {
                            } else if (mMatchErrorCounts % 2 == 0) {
                                mVib.vibrate(100);
                                dispatchMatchInfo(6,mMatchErrorCounts,1);
                            } else {
                                dispatchMatchInfo(6,mMatchErrorCounts,0);
                            }*/
						mHandler.sendEmptyMessage(MATCH_FAIL);
                        }else{
							waitFinger();
						} 			
                    }
                }else{
					waitFinger();
                }
                break;

            case SHUTDOWN_WHAT:
                if (DEBUG)
                    Log.d(TAG, "SHUTDOWN_WHAT");
				//mHandler.removeMessages(MATCH_WHAT);
                //mHandler.removeMessages(MATCH_AGAIN);
				writeGeneralKeycode();
                break;

            case POWERDOWN_WHAT:
                if (DEBUG)
                    Log.d(TAG, "POWERDOWN_WHAT");
                //mFpNative.FpPowerDown();
                break;

            case ISFINGERUP_WHAT:
                if (DEBUG)
                    Log.d(TAG, "ISFINGERUP_WHAT");
                int isUp = mFpNative.FpIsFingerUp();
                try {
                    if (mClient != null) {
                        mClient.getValue(ISFINGERUP_WHAT, isUp);
                    } else {
                        Log.e(TAG, "isFingerUp---is null---");
                    }
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                break;

            case MATCH_AGAIN:
                if (DEBUG)
                    Log.d(TAG, "MATCH_AGAIN");
                mHandler.removeMessages(MATCH_WHAT);
                mHandler.removeMessages(MATCH_AGAIN);

                String dir = fingerDataManager.FP_DATA_DIR;
                fileindex = fingerDataManager.getFileIndex();
                tempLength = fingerDataManager.getAllTemplateLength();
                fileLength = fileindex.length;
				try {
				    if(isRing == true){	
                    				mFpNative.FpWriteKeycode(169); 
						if (mClient != null)
							mClient.getValue(MATCH_AGAIN, -2015);
						break;
				    }
				} catch (RemoteException e1) {
		        	e1.printStackTrace();
		        }
                result = mFpNative.FpMatchFingerprint(dir, fileindex, tempLength, fileLength);
				
                if (DEBUG)
                    Log.d(TAG, "matchUp---result---" + result[0]);
				
				try {
                    if (mClient != null) {
                        mClient.getValue(MATCH_AGAIN, result[0]);
                    } else {
                        Log.e(TAG, "matchUp---is null---");
                    }
                } catch (RemoteException e1) {
                    e1.printStackTrace();
                }
				
				if(-1 == result[0]){
					acquireWakeLock();
					fingerup_counts = 0;
					while(-1 == mFpNative.FpIsFingerUp() && fingerup_counts++ < MAX_CHECK_FINGERUP);				
					releaseWakeLock();
				}
				writeGeneralKeycode();				

                break;
            case MATCH_SUCCESS:
                if (DEBUG)
                    Log.d(TAG, "MATCH_SUCCESS");
				//mHandler.removeMessages(MATCH_WHAT);
                //mHandler.removeMessages(MATCH_AGAIN);                
                unLockScreen();
                lightScreen();
				Settings.System.putInt(getContentResolver(), "com_btlfinger_fingerprint_unlock_flag", 1);
				acquireWakeLock();
				fingerup_counts = 0;
				while(-1 == mFpNative.FpIsFingerUp() && fingerup_counts++ < MAX_CHECK_FINGERUP);				
				releaseWakeLock();
				playing = false;
                //FingerLunchApp(msg.arg1);
                break;
            case MATCH_FAIL:
                if (DEBUG)
                    Log.d(TAG, "MATCH_FAIL");

				acquireWakeLock();
				fingerup_counts = 0;
                mHandler.sendEmptyMessage(MATCH_CHECKUPS);
                break;
            case MATCH_CHECKUPS:
                if (DEBUG)Log.d(TAG, "MATCH_CHECKUPS");
                mFingerprintUp = mFpNative.FpIsFingerUp();
                if (DEBUG)Log.d(TAG, "mFingerprintUp"+mFingerprintUp);
                if (mFingerprintUp == -1 && fingerup_counts++ < MAX_CHECK_FINGERUP) { // fingerdown
                    if (DEBUG) Log.i(TAG, "fingerdown");
                    if (mMatchType != 0){
                    mHandler.sendEmptyMessage(MATCH_CHECKUPS);
					}
					else{
						waitFinger();
					}
                } else {
                    if (DEBUG) Log.i(TAG, "fingerup");

					if (mMatchType == 2){//screen on match
						dispatchMatchInfo(5,mMatchErrorCounts,1);
					}
					waitFinger();
                }
                break;
            case MATCH_CHECKUPE:
                if (DEBUG)Log.d(TAG, "MATCH_CHECKUPE");
                if (mMatchType == 1)
					mFpNative.FpWaitScreenOn();
                    //releaseWakeLock();// goto sleep
                break;
			case MII_TEST:
                if (DEBUG)
                    Log.d(TAG, "MII_TEST");
                mHandler.removeMessages(MATCH_WHAT);
                mHandler.removeMessages(MATCH_AGAIN);

				int ret = mFpNative.FpMmiFpTest();
				playing = false;
				writeGeneralKeycode();

				try {
                    if (mClient != null) {
                        mClient.getValue(MII_TEST, ret);
                    } else {
                        Log.e(TAG, "matchUp---is null---");
                    }
                } catch (RemoteException e1) {
                    e1.printStackTrace();
                }
                break;
			case ISFINGERUP:
                if (DEBUG)
                    Log.d(TAG, "ISFINGERUP +++");
                Settings.System.putInt(getContentResolver(), "com_btlfinger_fingerprint_up_flag", -1);
				acquireWakeLock();
				fingerup_counts = 0;
				while(-1 == mFpNative.FpIsFingerUp() && fingerup_counts++ < MAX_CHECK_FINGERUP);  
				releaseWakeLock();
				playing = false;
				mFpNative.FpWaitScreenOn(); 
				Settings.System.putInt(getContentResolver(), "com_btlfinger_fingerprint_up_flag", 0);
                break;

		case FINGER_FUCTION:
                if (DEBUG)
                    Log.d(TAG, "FINGER_FUCTION");

	            int validApp = 0; //0 : none, 1:camera,2:music; 3: reader; 4:
	         		
	         	if (isCameraAppLaunch())validApp = 1;
	         	else if(isMusicAppLaunch())validApp = 2;
	
         		String topPackageName = getTopActivity().getPackageName();
         		String topClassName = getTopActivity().getClassName(); 

         		Log.d(TAG,"topPackageName: "+ topPackageName + "  topClassName: "+ topClassName);
         		
         		Log.d(TAG,"validApp: "+ validApp);
         		
         		if (isFunctionEnable(validApp)) {
         			Log.d(TAG, "isFunctionEnable: yes111111 ");
					
					if(0 == mFpNative.FpIsFingerUp()){
						Log.d(TAG, "AAAAAAA ");
						playing = false;
						mFpNative.FpWaitScreenOn();
						break;
					}

     				if (validApp == 1) {
     					keyRemappingSendFakeKeyEvent(KeyEvent.ACTION_DOWN,
     							KeyEvent.KEYCODE_CAMERA);
     					keyRemappingSendFakeKeyEvent(KeyEvent.ACTION_UP,
     							KeyEvent.KEYCODE_CAMERA);
     				} else if (validApp == 2) {
     					keyRemappingSendFakeKeyEvent(KeyEvent.ACTION_DOWN,
     							KeyEvent.KEYCODE_MEDIA_NEXT);
     					keyRemappingSendFakeKeyEvent(KeyEvent.ACTION_UP,
     							KeyEvent.KEYCODE_MEDIA_NEXT);
     				} else if (validApp == 3) {
     					keyRemappingSendFakeKeyEvent(KeyEvent.ACTION_DOWN,
     							KeyEvent.KEYCODE_VOLUME_DOWN);
     					keyRemappingSendFakeKeyEvent(KeyEvent.ACTION_UP,
     							KeyEvent.KEYCODE_VOLUME_DOWN);
     				} else if (validApp == 4) {
     					keyRemappingSendFakeKeyEvent(KeyEvent.ACTION_DOWN,
     							KeyEvent.KEYCODE_F12);
     					keyRemappingSendFakeKeyEvent(KeyEvent.ACTION_UP,
     							KeyEvent.KEYCODE_F12);
     				}
     				//Intent fingerprintIntent = new Intent(FINGER_PRINT_MATCH_ACTION);
     				//mContext.sendBroadcast(fingerprintIntent);

					acquireWakeLock();
					fingerup_counts = 0;
					Log.d(TAG, "+++++++++++ mFpNative.FpIsFingerUp():  " + mFpNative.FpIsFingerUp());
					//while(-1 == mFpNative.FpIsFingerUp() && fingerup_counts++ < MAX_CHECK_FINGERUP);
					while(-1 == mFpNative.FpIsFingerUp())Log.d(TAG, "333333333");  
					releaseWakeLock();
					playing = false;
					mFpNative.FpWaitScreenOn();
					Log.d(TAG, "-----------");
         		}else{
					playing = false;
				} 								
                break;


            default:
                break;
            }
            return true;
        }
    };

    public void bindService(Context context) {
        Intent intent = new Intent();
        intent.setClassName(KEYGUARD_PACKAGE, KEYGUARD_CLASS);
        if (!context.bindService(intent, mKeyguardConnection, Context.BIND_AUTO_CREATE)) {
            if (DEBUG)
                Log.v(TAG, "*** bindService: can't bind to " + KEYGUARD_CLASS);
        } else {
            if (DEBUG)
                Log.v(TAG, "*** bindService started");
        }
    }

    private final ServiceConnection mKeyguardConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            if (DEBUG)
                Log.v(TAG, "*** Keyguard connected (yay!)");
            mKeyguardService = IKeyguardService.Stub.asInterface(service);

        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            if (DEBUG)
                Log.v(TAG, "*** Keyguard disconnected (boo!)");
            mKeyguardService = null;
        }
    };


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // TODO Auto-generated method stub
        // return super.onStartCommand(intent, flags, startId);
        super.onStartCommand(intent, flags, startId);
        if (DEBUG)
            Log.d(TAG, "onStartCommand");

        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        if (DEBUG)
            Log.i(TAG, "onDestroy");

        try {
            String action = "com.btlfinger.aidlservice";
            Intent intent = new Intent(action);
            intent.setPackage("com.btlfinger.fingerprintunlock");
            startService(intent);
        } catch (Exception e) {
            e.printStackTrace();
        }

        super.onDestroy();
    }

    public int nVibratorNumber = 0;

    public void VibratorWhenWrong() {

        boolean isScreenOn = mPowerManager.isScreenOn();
        if(mUserPresent == false && isScreenOn == false ) {
            Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
            long[] pattern = { 100, 100 };
            vibrator.vibrate(pattern, -1);
        }


    }

    private void dispatchMatchInfo(int max, int counts, int error_on) {
        //sendBroadcast
        Intent intent = new Intent();
        intent.setAction(FINGER_PRINT_UNMATCH_ACTION);
        intent.putExtra("error_count_max", max);
        intent.putExtra("error_count", counts);
        intent.putExtra("error_on",error_on);
        sendBroadcast(intent);
    }


    public void getFingeprintTemplateInfo(int [] fileindex, int[] tempLength) {
        fileindex =  fingerDataManager.getFileIndex();
        tempLength = fingerDataManager.getAllTemplateLength();
    }


    public void lightScreen() {
        releaseWakeLock();
        if (mLightLock != null)
            mLightLock.acquire();
        if (mLightLock != null && mLightLock.isHeld())
            mLightLock.release();
    }

    public void releaseWakeLock() {
        if (mWakeLock != null && mWakeLock.isHeld())
            mWakeLock.release();
    }
	public void acquireWakeLock() {
        if (mWakeLock != null && !mWakeLock.isHeld())
			mWakeLock.acquire();
    }


    public void writeGeneralKeycode() {
        /*try {
            String strkeycode = String.valueOf(keycode);
            Log.d(TAG, "PUT_KEYCODE strkeycode---is ---" + strkeycode);
            BufferedWriter writer;

            writer = new BufferedWriter(new FileWriter(SYS_PATH));
            writer.write(strkeycode);
            writer.close();

        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }*/

		boolean isScreenOn = mPowerManager.isScreenOn();
		boolean isKeygurad = mKeyguardManager.inKeyguardRestrictedInputMode();
		
		mMatchErrorCounts = 0;
		if(isScreenOn){
			if(!isKeygurad){
				mMatchType = 0;
				if(!mtest){
					clearCallKeycode();
				}else{
					mFpNative.FpWriteKeycode(68);	
				}				
			}else{
				mMatchType = 2;	
				mFpNative.FpWriteKeycode(68);
			}	
		}else{	
			mMatchType = 1;	
			mFpNative.FpWriteKeycode(68);
		}
		mFpNative.FpWaitScreenOn(); 		
    }


    public void writeInterruptKeycode(int interruptcode) {
        try {

            String strinterruptcode = String.valueOf(interruptcode);
            Log.d(TAG, "writeInterruptKeycode strkeycode---is ---" + strinterruptcode);
            BufferedWriter writer;

            writer = new BufferedWriter(new FileWriter("/sys/bl229x_sysfs/key_interrupt"));
            writer.write(strinterruptcode);
            writer.close();

        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }


    public boolean writeCallKeycode() {
        String KEYCODE_CALL = "169";
        String KEYCODE_F11 = "87";

        if ((PreferenceUtils.isSettingFpCallLockOn())) {            
            if (DEBUG)Log.d(TAG, "write keycode_call before");
            mFpNative.FpWriteKeycode(169);                
            } else
            mFpNative.FpWriteKeycode(68);                            
	    	mFpNative.FpWaitScreenOn(); 
            // TODO Auto-generated catch block
        return true;
    }

    public boolean clearCallKeycode() {
        String KEYCODE_CALL = "169";
        String KEYCODE_F11 = "87";

        mFpNative.FpWriteKeycode(68);    

        return true;
    }

	public void waitFinger() {
		playing = false;
        mFpNative.FpWriteKeycode(68);
	    mFpNative.FpWaitScreenOn();
		releaseWakeLock();
    }

    public boolean unLockScreen() {
        try {
            mKeyguardService.keyguardDone(false, true);

        } catch (RemoteException e) {
            Log.d(TAG," unLock RemoteException when keyguardDone");
            e.printStackTrace();
        }
		//mLock.disableKeyguard();
        return true;
    }

    private List<FingerPrintModel> mFingerPrintModels = new ArrayList<FingerPrintModel>();
    public void FingerLunchApp(int preMatchFingerIndex) {
        Log.i(TAG, "Enter FingerLunchApp");
        mFingerPrintModels.clear();

        try {
            Cursor mCursor = queryAllFinger();
            mFingerPrintModels = getFingerPrintFromCursor(mCursor);
            FingerPrintModel model = null;

            Log.i(TAG, "Enter FingerLunchApp match index= "+ preMatchFingerIndex);
            if(preMatchFingerIndex >= 0) {
                model = mFingerPrintModels.get(preMatchFingerIndex);
            }

            if( model != null ) {
                if(model.fp_lunch_package != null ) {
                    Intent intent = getPackageManager().getLaunchIntentForPackage(model.fp_lunch_package);
                    startActivity(intent);
                }
            }

        } catch(Exception e) {
            e.printStackTrace();
        }

    }

    private Cursor queryAllFinger() {
        Log.e(TAG, "queryAllFinger...begin");
        Uri FPURI = Uri.parse("content://com.btlfinger.fingerprint.provider/fp_data_table");
        Cursor cursor = getContentResolver().query(FPURI, null, null, null, null);
        Log.i(TAG, "queryAllFinger...end; count = " + cursor.getCount());
        return cursor;
    }

    private List<FingerPrintModel> getFingerPrintFromCursor(Cursor cursor) {
        Log.e(TAG, "getFingerPrintFromCursor...begin");
        List<FingerPrintModel> fingerList = new ArrayList<FingerPrintModel>();
        while (cursor.moveToNext()) {
            FingerPrintModel finger = new FingerPrintModel();
            finger.fp_id = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_ID));
            finger.fp_name = cursor.getString(cursor.getColumnIndex(FpsTable.COL_FP_NAME));
            finger.fp_data_index = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_FP_DATA_INDEX));
            finger.fp_data_path = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_FP_DATA_PATH));
            finger.fp_lunch_package = cursor.getString(cursor.getColumnIndex(FpsTable.COL_FP_LUNCH_PACKAGE));

            try {

                Log.i(TAG, "finger.fp_id" + finger.fp_id);
                Log.i(TAG, "finger.fp_name" + finger.fp_name);
                Log.i(TAG, "finger.fp_lunch_package" + finger.fp_lunch_package);
            } catch(Exception e) {
            }
            fingerList.add(finger);
        }
        Log.e(TAG, "getFingerPrintFromCursor success...");
        return fingerList;
    }


	private ComponentName getTopActivity() {
        //final ActivityManager amTemp = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);		
        final ActivityManager amTemp = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
		ComponentName cn = amTemp.getRunningTasks(1).get(0).topActivity;
        return cn;
    }


	private String[] mCameraApps = { "com.android.camera",
			"com.android.gallery3d" };

	private boolean isCameraAppLaunch() {
		String topPackageName = getTopActivity().getPackageName();
		for (int i = 0; i < mCameraApps.length; i++) {
			if (topPackageName != null
					&& mCameraApps[i].contains(topPackageName)) {
				return true;
			}
		}
		return false;
	}

	private String[] mReaderApps = { "com.changdu", "com.qq.reader",
			"com.dangdang.reader", "com.shuqi.controller", "com.tencent.weread","com.chaozh.iReaderFree","com.netease.pris"};

	private boolean isReaderAppLaunch() {
		String topPackageName = getTopActivity().getPackageName();
		for (int i = 0; i < mReaderApps.length; i++) {
			if (topPackageName != null
					&& mReaderApps[i].contains(topPackageName)) {
				return true;
			}
		}
		return false;
	}

	private String[] mMusicApps = { "com.tencent.qqmusic", "com.duomi.android",
			"cmccwm.mobilemusic", "fm.xiami.main", "com.netease.cloudmusic",
			"com.sds.android.ttpod", "com.kugou.android",
			"com.tencent.karaoke", "com.android.music", "cn.kuwo.player" };

	private boolean isMusicAppLaunch() {
		String topPackageName = getTopActivity().getPackageName();
		for (int i = 0; i < mMusicApps.length; i++) {
			if (topPackageName != null
					&& mMusicApps[i].contains(topPackageName)) {
				return true;
			}
		}
		return false;
	}

	private boolean isLauncherAppLaunch() {
		String topPackageName = getTopActivity().getPackageName();
		String launcherPackageName = "com.android.launcher3";
		for (int i = 0; i < mMusicApps.length; i++) {
			if (topPackageName != null
					&& launcherPackageName.contains(topPackageName)) {
				return true;
			}
		}
		return false;
	}

	private boolean isFunctionEnable(int functionID) {
		boolean fEnable = false;
		if (functionID == 1) // camera
			fEnable = Settings.System.getInt(mContext.getContentResolver(),
					Settings.System.BTLFINGER_FINGERPRINT_USEDTO_CAMERA, 0) == 1 ? true
					: false;
		else if (functionID == 2) // music
			fEnable = Settings.System.getInt(mContext.getContentResolver(),
					Settings.System.BTLFINGER_FINGERPRINT_USEDTO_MUSIC, 0) == 1 ? true
					: false;
		else if (functionID == 3) // reader
			fEnable = Settings.System.getInt(mContext.getContentResolver(),
					Settings.System.BTLFINGER_FINGERPRINT_USEDTO_READER, 0) == 1 ? true
					: false;
		else if (functionID == 4)
			fEnable = Settings.System.getInt(mContext.getContentResolver(),
					Settings.System.BTLFINGER_FINGERPRINT_USEDTO_DESKTOP, 0) == 1 ? true
					: false;
		return fEnable;
	}

	private long mKeyRemappingSendFakeKeyDownTime;
    	private void keyRemappingSendFakeKeyEvent(int action, int keyCode) {
        long eventTime = SystemClock.uptimeMillis();
        if (action == KeyEvent.ACTION_DOWN) {
            mKeyRemappingSendFakeKeyDownTime = eventTime;
        }

        KeyEvent keyEvent
                = new KeyEvent(mKeyRemappingSendFakeKeyDownTime, eventTime, action, keyCode, 0);
        InputManager inputManager = (InputManager) mContext.getSystemService(Context.INPUT_SERVICE);
        inputManager.injectInputEvent(keyEvent, InputManager.INJECT_INPUT_EVENT_MODE_ASYNC);
	}


    /*
     * class MonitorPhoneState extends PhoneStateListener {
     *
     * @Override public void onCallStateChanged(int state, String
     * incomingNumber) { Log.i(TAG, "onCallStateChanged"); switch (state) { case
     * TelephonyManager.CALL_STATE_RINGING: Log.i(TAG, "CALL_STATE_RINGING");
     * break; default: break; } super.onCallStateChanged(state, incomingNumber);
     * } }
     */
}
