package com.btlfinger.service;

import com.btlfinger.fingerprint.FingerPrintManager;
import com.btlfinger.fingerprint.FingerPrintModel;
import com.btlfinger.fingerprint.FpNative;
import com.btlfinger.fingerprint.dao.DbHelper;
import com.btlfinger.fingerprint.dao.FpsTable;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
import com.btlfinger.aidl.IFpsFingerClient;
import com.btlfinger.aidl.IFpsFingerManager;
import com.btlfinger.fingerprintunlock.PackagesConstant;


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

    private TelephonyManager mTelemanager;
    private PowerManager mPowerManager;
    private KeyguardManager mKeyguardManager;
	private PowerManager.WakeLock mWakeLock;
	private TelephonyManager tm;
	public static final int MII_TEST = 19;
	public static final int MAX_CHECK_FINGERUP = 100;
	public static final int FINGER_FUCTION= 20;
 
    private FpNative mFpNative = null;
    private IFpsFingerClient mClient;
	private boolean mtest = false;
	private boolean mIsRing = false;
	public int fingerup_counts = 0;
    private Context mContext;
    private final String SYS_PATH = "/sys/bl229x_sysfs/keycode";

    @Override
    public IBinder onBind(Intent arg0) {
        return mBinder;
    }

    private FingerPrintManager fingerDataManager = null;
    private static String DEFAULT_FP_NAME = "Fingerprint";

    private HandlerThread mFingerThread = new HandlerThread("FingerThread");
    private Handler mHandler = null;
  
    private final IFpsFingerManager.Stub mBinder = new IFpsFingerManager.Stub() {

        @Override
        public int SetKeyCode(int keycode) throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "getValue");

            return 0;
        }

        @Override
        public void waitScreenOn() throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "waitScreenOn");
            mFpNative.FpWaitScreenOn();
        }
        @Override
        public void listen(IFpsFingerClient client) throws RemoteException {
            if (DEBUG)
                Log.i(TAG, "listen");
            mClient = client;
        }
		@Override
		public int mmiFpTest() throws RemoteException {		
			Log.i(TAG, "mmiFpTest");
			int ret = 0;

			//Log.i(TAG, "mmiFpTest start");
			//ret = mFpNative.FpMmiFpTest();
			mtest = true;

			mFpNative.FpWaitScreenOn();
	
			return ret;
		}
    };

    @Override
    public void onCreate() {

        super.onCreate();
        if (DEBUG)
            Log.i(TAG, "FingerService onCreate");

        mFpNative = new FpNative();
		
		mFingerThread.start();
		mHandler = new Handler(mFingerThread.getLooper(), callback);
        IntentFilter filter = new IntentFilter();
        // listen screen_on/off event

        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(FINGER_PRINT_MATCH_ACTION);
        filter.setPriority(Integer.MAX_VALUE);
		registerReceiver(mScreenReceiver, filter);
		
		mKeyguardManager = (KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE);
		mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
		mWakeLock = mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
                                              this.getClass().getCanonicalName());
        tm = (TelephonyManager)getSystemService(Service.TELEPHONY_SERVICE);

        tm.listen(phonestatelistener, PhoneStateListener.LISTEN_CALL_STATE);

        bindService(this);
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
				mIsRing = false;
                clearCallKeycode();
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
                    mIsRing = true;
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
           /* boolean isScreenOn = mPowerManager.isScreenOn();
            boolean isKeygurad = mKeyguardManager.inKeyguardRestrictedInputMode();
            
             Log.d(TAG, "onReceive action111111111 = " + action);
             
             Log.d(TAG, "isScreenOn222222 = " + isScreenOn);
             Log.d(TAG, "isKeygurad333333 = " + isKeygurad);
             Log.d(TAG, "playing 444444444= " + playing);*/
			Log.d(TAG, "onReceive action111111111 = " + action);
            
            
            if (action.equals(FINGER_PRINT_MATCH_ACTION) && !playing) {  
            	playing = true;
            	if (DEBUG)
	                Log.d(TAG, "onReceive action = " + action);
	
	            if (DEBUG)
	            	Log.d(TAG, "onReceive FINGER_PRINT_MATCH_ACTION");
	            
				if(mIsRing){
					Log.d(TAG, "1111111111111111111111111111111111111111");
					mIsRing = false;
					tm.answerRingingCall();
					playing = false;
					return;
				}

				if(!mPowerManager.isScreenOn()){
					playing = false;
					return;
				}
				
				if(mtest && !mKeyguardManager.inKeyguardRestrictedInputMode()){
					mtest = false;
					mHandler.sendEmptyMessage(MII_TEST);
					return;
				}
				
	            mContext = context;
				mHandler.sendEmptyMessage(FINGER_FUCTION);     
            }else if (action.equals(Intent.ACTION_SCREEN_OFF) ) {
                if (DEBUG)
                    Log.d(TAG, "onReceive ACTION_SCREEN_OFF");
                Settings.System.putString(getContentResolver(), "com_btlfinger_fingerprint_lastapp_package", "com.btlfinger.fingerprintunlock");
            } 
        }
    };

    private Callback callback = new Callback() {
        String dir;
        int[] result;
        int[] fileindex;
        int[] tempLength;
        int fileLength;
        int score;
        @Override
        public boolean handleMessage(Message msg) {
            switch (msg.what) {
			case MII_TEST:
                if (DEBUG)
                    Log.d(TAG, "MII_TEST");

				/*int ret = mFpNative.FpMmiFpTest();
				mFpNative.FpWaitScreenOn();

				try {
                    if (mClient != null) {
                        mClient.getValue(MII_TEST, ret);
                    } else {
                        Log.e(TAG, "matchUp---is null---");
                    }
                } catch (RemoteException e1) {
                    e1.printStackTrace();
                }*/
				playing = false;
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
         			Log.d(TAG, "isFunctionEnable: yes ");
	
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
					while(-1 == mFpNative.FpIsFingerUp() && fingerup_counts++ < MAX_CHECK_FINGERUP);  
					releaseWakeLock();
					playing = false;
					mFpNative.FpWaitScreenOn();
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

        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            if (DEBUG)
                Log.v(TAG, "*** Keyguard disconnected (boo!)");
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
    }
    public boolean writeCallKeycode() {
        int KEYCODE_CALL = 169;
 
        mFpNative.FpWriteKeycode(KEYCODE_CALL);
        return true;
    }

    public boolean clearCallKeycode() {
        int KEYCODE_F10= 68;

        mFpNative.FpWriteKeycode(KEYCODE_F10);
        return true;
    }

	 public void releaseWakeLock() {
        if (mWakeLock != null && mWakeLock.isHeld())
            mWakeLock.release();
    }
	public void acquireWakeLock() {
        if (mWakeLock != null && !mWakeLock.isHeld())
			mWakeLock.acquire();
    }
    
    private ComponentName getTopActivity() {
        //final ActivityManager amTemp = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);		
        final ActivityManager amTemp = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
		ComponentName cn = amTemp.getRunningTasks(1).get(0).topActivity;
        return cn;
    }
	private final String[] mMusicApps = {
        "com.tencent.qqmusic",
        "com.duomi.android",
        "cmccwm.mobilemusic",
        "fm.xiami.main",
        "com.netease.cloudmusic",
        "com.sds.android.ttpod",
        "com.kugou.android",
        "com.tencent.karaoke",
        "com.android.music",
        "cn.kuwo.player",
		"com.google.android.music"
    };
	
	private boolean isMusicAppLaunch() {
        String topPackageName = getTopActivity().getPackageName();
		if(topPackageName != null){
			String appString1 = Settings.System.BTLFINGER_FINGERPRINT_MUSIC_RES1;
			String appString2 = Settings.System.BTLFINGER_FINGERPRINT_MUSIC_RES2;
			String appString3 = Settings.System.BTLFINGER_FINGERPRINT_MUSIC_RES3;
			if ((appString1!=null&&appString1.contains(topPackageName)) || (appString2!=null&&appString1.contains(topPackageName)) || 				(appString3!=null&&appString1.contains(topPackageName))) {
		        return true;
		    }
		
		    for (int i = 0; i< mMusicApps.length; i++) {
		        if (topPackageName != null && mMusicApps[i].contains(topPackageName)) {
				    return true;
		        }
		    }
		
		}
		
        return false;
	} 
    
	private final String[] mCameraApps = {
        "com.android.camera",
        "com.android.gallery3d",
		"com.mediatek.camera"
	};
	
	private boolean isCameraAppLaunch() {
        String topPackageName = getTopActivity().getPackageName();
		if(topPackageName != null){
			String appString1 = Settings.System.BTLFINGER_FINGERPRINT_CAMERA_RES1;
			String appString2 = Settings.System.BTLFINGER_FINGERPRINT_CAMERA_RES2;
			String appString3 = Settings.System.BTLFINGER_FINGERPRINT_CAMERA_RES3;
			if ((appString1!=null&&appString1.contains(topPackageName)) || (appString2!=null&&appString1.contains(topPackageName)) || 				(appString3!=null&&appString1.contains(topPackageName))) {
		        return true;
		    }
		
		    for (int i = 0; i< mCameraApps.length; i++) {
		        if (topPackageName != null && mCameraApps[i].contains(topPackageName)) {
		           return true;
				}
		    }
		
		}

        return false;
    }
	
  
	
	private boolean isFunctionEnable(int functionID) {
        boolean fEnable = false;
        if (functionID == 1)       // camera
            fEnable = Settings.System.getInt(mContext.getContentResolver(), Settings.System.BTLFINGER_FINGERPRINT_USEDTO_CAMERA, 0)== 1 ? true:false;
        else if (functionID == 2)  // music
            fEnable = Settings.System.getInt(mContext.getContentResolver(), Settings.System.BTLFINGER_FINGERPRINT_USEDTO_MUSIC, 0)== 1 ? true:false;
        else if (functionID == 3)  // reader
            fEnable = Settings.System.getInt(mContext.getContentResolver(), Settings.System.BTLFINGER_FINGERPRINT_USEDTO_READER, 0)== 1 ? true:false;
        else if (functionID == 4)  // desktop
            fEnable = Settings.System.getInt(mContext.getContentResolver(), Settings.System.BTLFINGER_FINGERPRINT_USEDTO_DESKTOP, 0)== 1 ? true:false;
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
