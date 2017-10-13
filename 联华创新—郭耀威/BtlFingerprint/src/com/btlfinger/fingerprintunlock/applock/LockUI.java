package com.btlfinger.fingerprintunlock.applock;

import com.btlfinger.fingerprintunlock.PackagesConstant;
import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.fingerprintunlock.applock.NumberPad.OnNumberClickListener;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
//import com.btlfinger.fingerprintunlock.ui.support.AddFpActivity;
import com.btlfinger.fingerprintunlock.ui.support.ManagerPINActivity;
import com.btlfinger.fingerprintunlock.ui.support.PasswdActivity;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.app.ActionBar;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.app.ActivityManager;

import android.os.CancellationSignal;
import android.hardware.fingerprint.Fingerprint;
import android.hardware.fingerprint.FingerprintManager;
import android.hardware.fingerprint.FingerprintManager.AuthenticationCallback;
import android.hardware.fingerprint.FingerprintManager.AuthenticationResult;

import com.btlfinger.fingerprint.FingerPrintManager;
//import com.btlfinger.service.aidl.IFpsFingerClient;
//import com.btlfinger.service.aidl.IFpsFingerManager;
import android.os.RemoteException;

import com.btlfinger.fingerprintunlock.support.ChooseLockSettingsHelper;

/**
 * 指纹详情
 *
 * @author blestech
 * @since 2015-11-26
 */

public class LockUI extends Activity implements OnNumberClickListener,
    OnClickListener {
    private ProcessIndication mIndication = null;
    private NumberPad mNumberPad = null;
    private StringBuilder mPassword = null;
    private String TAG = "LockUI";
    private String mGlobalPackageNameString = null;
    private final int BIRD_FINGER_SEND_METCH = 1;

    public static String SYSTEM_SETTINGS_FP_SCREENLOCK = "com_btlfinger_fingerprint_usedto_screenlock";
    public static String SYSTEM_SETTINGS_FP_APPLOCK = "com_btlfinger_fingerprint_usedto_applock";
    public static String SYSTEM_SETTINGS_FP_CAMERALOCK = "com_btlfinger_fingerprint_usedto_camera";
    public static String SYSTEM_SETTINGS_FP_MUSICLOCK = "com_btlfinger_fingerprint_usedto_music";
    public static String SYSTEM_SETTINGS_FP_CALLLOCK = "com_btlfinger_fingerprint_usedto_call";
    public static String SYSTEM_SETTINGS_FP_DESKTOPLOCK= "com_btlfinger_fingerprint_usedto_desktop";
    public static String SYSTEM_NEEDLOCKAPP_PACKAGE_NAME = "com_btlfinger_fingerprint_needlockapp_package";

    private static final int MATCH_SUCCESS = 1;
    private static final int MATCH_FAIL = 2;
    private static final int MATCH_AGAIN = 3;
    private static final int MATCH_FINISH = 4;

    private boolean lockoutFlag = false;
    private boolean mFlag = false;
    private int failCount = 0;
    private static final int MAXFAILLIMIT = 5;// 控制最大失败次数，大于此次数就跳转到密码输入界面

    private Button btn_toDigital = null;

    private ViewGroup fingerLayout = null;
	private ViewGroup digitaLayout = null;
    
    private CancellationSignal mFingerprintCancelSignal = null;
    private FingerprintManager mFpm = null;
    private FingerprintManager.AuthenticationCallback mAuthenticationCallback
    = new AuthenticationCallback() {

		@Override
		public void onAuthenticationFailed() {
			Log.i(TAG, "11111111111111111111");
			lockUIHandler.sendEmptyMessage(MATCH_FAIL);
		};
		
		@Override
		public void onAuthenticationSucceeded(AuthenticationResult result) {
			lockUIHandler.sendEmptyMessage(MATCH_SUCCESS);
		}
		
		@Override
		public void onAuthenticationHelp(int helpMsgId, CharSequence helpString) {
		    //handleFingerprintHelp(helpMsgId, helpString.toString());
		}
		
		@Override
		public void onAuthenticationError(int errMsgId, CharSequence errString) {
			
		}
		
		@Override
		public void onAuthenticationAcquired(int acquireInfo) {
			//lockUIHandler.sendEmptyMessage(MATCH_FAIL);
		}
    };
    
    /*private final FingerprintManager.LockoutResetCallback mLockoutResetCallback
    = new FingerprintManager.LockoutResetCallback() {
		@Override
		public void onLockoutReset() {
			Log.i(TAG, "555555555555555");
		    lockoutFlag = false;
		}
    };*/

    //1111private IFpsFingerManager fm = null;
    /*1111private IFpsFingerClient client = new IFpsFingerClient.Stub() {

        @Override
        public void getValue(int type, int score) throws RemoteException {

            Log.i(TAG, "type = " + type + "---score = " + score);

            if (type == 3) {// 注册时候的返回值
                if (score > 0) {// 指纹对比成功
                    lockUIHandler.sendEmptyMessage(MATCH_SUCCESS);

                } else if (score != -2047) {
                    Log.i(TAG, "---start another match---");
                    Message msg = lockUIHandler.obtainMessage(MATCH_FAIL);
                    msg.arg1 = score;
                    lockUIHandler.sendMessage(msg);
                }
            }
        }
    };*/

    /*1111public void initService() {
        Log.i(TAG, "initService called");
        Intent intent = new Intent();
        intent.setAction("com.btlfinger.aidlservice");
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    public boolean bServiceConnected = false;
    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            // TODO Auto-generated method stub

            Log.i(TAG, "on FingerService ServiceConnected");
            fm = IFpsFingerManager.Stub.asInterface(service);
            try {
                fm.listen(client);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            try {
				lockUIHandler.removeMessages(MATCH_AGAIN);
				fm.cancle(0);
                lockUIHandler.sendEmptyMessage(MATCH_AGAIN);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            bServiceConnected = true;

        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            // TODO Auto-generated method stub
            fm = null;

            bServiceConnected = false;
        }
    };*/

    private Handler lockUIHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MATCH_SUCCESS:
                Log.i(TAG, "PreferenceUtils.getLockUIStatus():" + PreferenceUtils.getLockUIStatus());
                if (PreferenceUtils.getLockUIStatus() == true) {// 如果解锁显示在当前界面
                    if (mGlobalPackageNameString != null) {
                        Settings.System
                        .putString(
                            getContentResolver(),
                            PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME,
                            mGlobalPackageNameString);
                    }

                    PreferenceUtils.setLockUIStatus(false);

                    failCount = 0;
                    Log.i(TAG, "finish:" );
                    finish();
                }
                break;

            case MATCH_FAIL:
                if (PreferenceUtils.getLockUIStatus() == true) {// 如果解锁显示在当前界面
                    //int score = msg.arg1;

                   //if ((score==-1) || (score==-100)) {// 修改为只是对比失败的时候才计数
                        failCount++;
                        mFpm.resetTimeout(null);
                        showToast(true);
                        Animation shake = AnimationUtils.loadAnimation(
                                              LockUI.this, R.anim.shake);// 加载动画资源文件
                        findViewById(R.id.id_lockui_fp_iv)
                        .startAnimation(shake); // 给组件播放动画效果
                   // }

                    if (failCount >= MAXFAILLIMIT) {
                        if (fingerLayout.getVisibility() != View.GONE) {
                            fingerLayout.setVisibility(View.GONE);
                            if (mFingerprintCancelSignal != null) {
                                mFingerprintCancelSignal.cancel();
                            }
							digitaLayout.setVisibility(View.VISIBLE);
                            //gotoManageF();
                            failCount = 0;
                            lockoutFlag = true;    
                        }
                    }

                   /*1111 try {
						if (mHasCancelAction == false)
                           fm.matchUp(1);
                    } catch (RemoteException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }*/

                }
                break;

            case MATCH_AGAIN:
               /*1111 try {
                    fm.matchUp(1);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }*/
                break;

            default:
                break;
            }

        }
    };

    private boolean mHasCancelAction = false;
    private int userId = 0;

    private BroadcastReceiver receiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)) {
            	Log.d(TAG, "22222222222222222222");
            	Settings.System.putString(getContentResolver(), PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME, "com.btlfinger.fingerprintunlock");
            }
        }
    };

    private void registerReceiver() {
    	Log.d(TAG, "33333333333333333333");
        IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
        filter.setPriority(Integer.MAX_VALUE);
        this.registerReceiver(receiver, filter);
    }

    /**
    * 开启PasswdActivity界面
    *
    * @param view
    */
    public void gotoManageF() {
        ChooseLockSettingsHelper temp = new ChooseLockSettingsHelper(this, null);
        temp.launchConfirmationActivity(100, null, null);
        return;
    }
    private void unRegisterReceiver() {
        this.unregisterReceiver(receiver);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);

        mHasCancelAction = false;
        Log.e(TAG, "onCreate");
        mGlobalPackageNameString = getIntent().getStringExtra("packagename");

        mPassword = new StringBuilder();


        setContentView(R.layout.activity_lock_ui);

        ActionBar.LayoutParams lp = new ActionBar.LayoutParams(
            ActionBar.LayoutParams.MATCH_PARENT,
            ActionBar.LayoutParams.MATCH_PARENT, Gravity.CENTER);

        View viewTitleBar = getLayoutInflater().inflate(
                                R.layout.lockui_actionbar_layout, null);
        ActionBar actionBar = getActionBar();
        actionBar.setCustomView(viewTitleBar, lp);
        actionBar.setDisplayShowHomeEnabled(false);// 去掉导航
        actionBar.setDisplayShowTitleEnabled(false);// 去掉标题
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setDisplayShowCustomEnabled(true);

        mIndication = (ProcessIndication) findViewById(R.id.id_lockui_indication);
        mNumberPad = (NumberPad) findViewById(R.id.id_lockui_numberpad);
        mNumberPad.setOnNumberClickListener(this);
        fingerLayout = (ViewGroup) findViewById(R.id.fingerlayout);
		digitaLayout = (ViewGroup) findViewById(R.id.digitallayout);
        btn_toDigital = (Button) findViewById(R.id.toDigital);

        btn_toDigital.setOnClickListener(this);
        userId = ActivityManager.getCurrentUser();
        
        mFpm = (FingerprintManager) getSystemService(Context.FINGERPRINT_SERVICE);
        /*if (mFpm != null) {
            mFpm.addLockoutResetCallback(mLockoutResetCallback);
        }*/
        
        /*if (mFingerprintCancelSignal != null) {
            mFingerprintCancelSignal.cancel();
        }
        mFingerprintCancelSignal = new CancellationSignal();
        mFpm.authenticate(null, mFingerprintCancelSignal, 0, mAuthenticationCallback, null, userId);
       // 1111initService();
        */
       //Log.e(TAG, "LockUI.getIntent() packageName:" + mGlobalPackageNameString);
      //registerReceiver();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
        case R.id.toDigital:
            fingerLayout.setVisibility(View.GONE);
			digitaLayout.setVisibility(View.VISIBLE);
            	
			if (mFingerprintCancelSignal != null) {
                mFingerprintCancelSignal.cancel();
            }
			//gotoManageF();
            /*if (mToast != null) {
                mToast.cancel();
            }*/
            break;
        default:
            break;
        }
    }

    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
	
        if (intent != null) {
            mGlobalPackageNameString = intent.getStringExtra("packagename");
			Log.d(TAG, "onNewIntent: " + intent + "packagename:"+ mGlobalPackageNameString);
        }
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
    	/*1111try {
			mHasCancelAction = false;
            //fm.cancle(0);
            fm.matchUp(1);
        } catch (Exception e) {
            e.printStackTrace();
        }*/
    	
    	  super.onResume();
          Log.d(TAG, "LockUI onResume");
          
          if (mGlobalPackageNameString.equals( PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME) || PreferenceUtils.getLockUIStatus()){
    		  finish();
    		  return;
    	 }
         PreferenceUtils.setLockUIStatus(true); 
          
         List<Fingerprint> items;
         int fingerprintCount = 0;
          if (mFpm != null) {
        	  items = mFpm.getEnrolledFingerprints();
              fingerprintCount = items != null ? items.size() : 0;
              if (fingerprintCount<=0 || !mFpm.isHardwareDetected()) {
     			   Log.i(TAG, "No fingerprint hardware detected!!");
     			   digitaLayout.setVisibility(View.VISIBLE);
     		       fingerLayout.setVisibility(View.GONE);
     		      mFlag = true;
     			   return;
              }
          }else{
        	  digitaLayout.setVisibility(View.VISIBLE);
		      fingerLayout.setVisibility(View.GONE);
		      mFlag = true;
		      return;
          }
          
          digitaLayout.setVisibility(View.GONE);
          fingerLayout.setVisibility(View.VISIBLE);        

        failCount = 0;
        String appString = Settings.System.getString(getContentResolver(), SYSTEM_NEEDLOCKAPP_PACKAGE_NAME);
        int appLockAble = Settings.System.getInt(getContentResolver(), Settings.System.BTLFINGER_FINGERPRINT_USEDTO_APPLOCK, 0);

        // 解决关闭应用锁以后还会上锁的问题
        boolean needFinish = true;
        if (appString != null) {
            String[] appsStrings = appString.split("\\|");
            for (int i = 0; i < appsStrings.length; i++) {
               // Log.d(TAG, "LockUI -- >appsStrings[i]" + appsStrings[i]);
               // Log.d(TAG, "appLockAble:" + appLockAble);
                if (mGlobalPackageNameString.equals(appsStrings[i])) {
                    needFinish = false;// 如果未在需要枷锁列表中
                    if(appLockAble != 0) {
						Log.d(TAG, "appLockAble:" + appLockAble);
						
						try {
							Thread.sleep(300);
						} catch (InterruptedException e) {
							e.printStackTrace(); 
						}

						if (mFingerprintCancelSignal != null) {
							mFingerprintCancelSignal.cancel();
						}
						mFingerprintCancelSignal = new CancellationSignal();
						
						//do{
							mFpm.authenticate(null, mFingerprintCancelSignal, 0, mAuthenticationCallback, null, userId);
						//}while(lockoutFlag);
						//1111initService();
                        return;
                    }
                }
            }
        }

        if ((PreferenceUtils.isSettingFpAppLockOn() == false || needFinish == true) || (needFinish == true ||appLockAble==0 )) {
        	finish();
        }
    }

    Toast mToast = null;

    public void showToast(boolean flag) {
        String text = null;
        if (flag) {
            text = getString(R.string.fp_tryagain);
        } else {
            text = getString(R.string.pwd_tryagain);
        }

        if (mToast == null) {
            mToast = Toast.makeText(LockUI.this, text, Toast.LENGTH_SHORT);
        } else {
            mToast.setText(text);
            mToast.setDuration(Toast.LENGTH_SHORT);
        }
        mToast.show();
    }

    /**
     * 显示剩余时间
     */
    public void showLeftTime() {
        if (mToast == null) {
            mToast = Toast.makeText(this, "请" + (10 - mWaitTime) + "秒后再试",
                                    Toast.LENGTH_SHORT);
        } else {
            mToast.setText("请" + (10 - mWaitTime) + "秒后再试");
            mToast.setDuration(Toast.LENGTH_SHORT);
        }
        mToast.show();
    }
    
    public void showNoPwd() {
        if (mToast == null) {
            mToast = Toast.makeText(this, getString(R.string.fp_no_pwd),
                                    Toast.LENGTH_SHORT);
        } else {
            mToast.setText( getString(R.string.fp_no_pwd));
            mToast.setDuration(Toast.LENGTH_SHORT);
        }
        mToast.show();
        
        Log.i(TAG, "3434242413414133");
        if (mFingerprintCancelSignal != null) {
           mFingerprintCancelSignal.cancel();
        }
        mFingerprintCancelSignal = new CancellationSignal();
    	mFpm.authenticate(null, mFingerprintCancelSignal, 0, mAuthenticationCallback, null, userId);
		
		 PreferenceUtils.setLockUIStatus(true);
        digitaLayout.setVisibility(View.GONE);
        fingerLayout.setVisibility(View.VISIBLE);
        failCount = 0;
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        Log.d(TAG, "LockUI onPause");
		Intent intent = new Intent();
        intent.setAction("com.blestech.fingerprint.lockui");
        sendBroadcast(intent);

        super.onPause();
        try {
            if (mHasCancelAction == false) {
            	 if (mFingerprintCancelSignal != null) {
                     mFingerprintCancelSignal.cancel();
                 }
				mHasCancelAction = true;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
		PreferenceUtils.setLockUIStatus(false);
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "LockUI onStop");
        // TODO Auto-generated method stub
        super.onStop();
        // PreferenceUtils.setLockUIStatus(false);


    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "LockUI onDestroy");
        // TODO Auto-generated method stub
        /*1111client = null;
		fm = null;
		unbindService(mConnection);
        mConnection = null;
        unRegisterReceiver();*/
        super.onDestroy();
        //Log.e(TAG, "LockUI -- >onDestroy");
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            //Intent i = new Intent(Intent.ACTION_MAIN);
           //i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            //i.addCategory(Intent.CATEGORY_HOME);
            //startActivity(i);
        	//onResume();
        	 Log.d(TAG, "KEYCODE_BACK:"+View.VISIBLE+":"+View.INVISIBLE+":"+digitaLayout.getVisibility()+":"+fingerLayout.getVisibility());
        	if(View.VISIBLE == digitaLayout.getVisibility() && View.VISIBLE  != fingerLayout.getVisibility() && !mFlag){
        		//onResume();
        		
        		if (mFingerprintCancelSignal != null) {
                    mFingerprintCancelSignal.cancel();
                }
                mFingerprintCancelSignal = new CancellationSignal();
            	mFpm.authenticate(null, mFingerprintCancelSignal, 0, mAuthenticationCallback, null, userId);
        		
        		 PreferenceUtils.setLockUIStatus(true);
    	        digitaLayout.setVisibility(View.GONE);
    	        fingerLayout.setVisibility(View.VISIBLE);
    	        failCount = 0;
        		
        		
        		return true;
        	}else{
        		Intent i = new Intent(Intent.ACTION_MAIN);
                i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                i.addCategory(Intent.CATEGORY_HOME);
                startActivity(i);
        	}
        }
        return super.onKeyDown(keyCode, event);
    }

    private boolean mPaswdFlag = false;
    private int mInputPwdCount = 0;// 输入密码的次数，超过5次则30秒内禁止输入密码
    private boolean mNberPadClickable = true;
    private int mWaitTime = 0;// 禁止输入密码后等待的时间

    @Override
    public void clickNumer(int number) {
        mPaswdFlag = true;
        if (number == -2) {
            /*
             * fingerLayout.setVisibility(View.VISIBLE);
             * digitaLayout.setVisibility(View.GONE); failCount = 0;
             *
             * if (mToast != null) { mToast.cancel(); }
             */
        } else if (number == -1) {
            // 如果点击删除键
            if (mPassword.length() > 0) {
                mPassword.deleteCharAt(mPassword.length() - 1);
            }

        } else {

            if (mNberPadClickable == true) {
                // 如果是其他数字按键
                mPassword.append(number);
                String pwd = mPassword.toString();
                if (pwd.equals(PreferenceUtils.getPwd())) {
                    mPaswdFlag = false;
                    PreferenceUtils.setLockUIStatus(false);
                    mPassword.delete(0, mPassword.length());
                    if (mGlobalPackageNameString != null) {
                        // 保存当前应用包名
                        Settings.System
                        .putString(
                            getContentResolver(),
                            PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME,
                            mGlobalPackageNameString);
                    }

                    mHasCancelAction = true;

                    /*1111try {
                        fm.cancle(0);
                    } catch (RemoteException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }*/

                    Log.e(TAG, "密码输入正确，所以取消操作 : ");
                    finish();
                } else if (pwd.length() == 4) {
                    mPaswdFlag = false;
                    showToast(false);
                    Animation shake = AnimationUtils.loadAnimation(LockUI.this,
                                      R.anim.shake);// 加载动画资源文件
                    mIndication.startAnimation(shake); // 给组件播放动画效果
                    mPassword.delete(0, mPassword.length());
                    mInputPwdCount += 1;
                    if (mInputPwdCount == 3) {
                        mNberPadClickable = false;
                        lockUIHandler.post(new Runnable() {

                            @Override
                            public void run() {
                                lockUIHandler.postDelayed(this, 1000);
                                mWaitTime += 1;
                                Log.e(TAG, "mWaitTime = " + mWaitTime);
                                if (mWaitTime == 10) {
                                    mWaitTime = 0;
                                    mInputPwdCount = 0;
                                    lockUIHandler.removeCallbacks(this);
                                    mNberPadClickable = true;
                                }
                            }
                        });
                    }
                }
            } else {
                showLeftTime();
            }
        }
        mIndication.setIndex(mPassword.length());
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (requestCode == 100 && resultCode == Activity.RESULT_OK) {
            Log.e(TAG, "onActivityResult");
            susecessAndExit();
        } else {
            try {
                mHasCancelAction = false;
                //fm.cancle(0);
                //fm.matchUp(1);
                if (mFingerprintCancelSignal != null) {
                    mFingerprintCancelSignal.cancel();
                }
                mFingerprintCancelSignal = new CancellationSignal();
                //do{
                	mFpm.authenticate(null, mFingerprintCancelSignal, 0, mAuthenticationCallback, null, userId);
               //}while(lockoutFlag);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
    
    public void susecessAndExit() {

    	PreferenceUtils.setLockUIStatus(false);
        if (mGlobalPackageNameString != null) {
            // 保存当前应用包名
            Settings.System
            .putString(
                getContentResolver(),
                PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME,
                mGlobalPackageNameString);
        }
        try {
            if (mHasCancelAction == false) {
                mHasCancelAction = true;
                //fm.cancle(0);
                //fm.powerDown();
                if (mFingerprintCancelSignal != null) {
                    mFingerprintCancelSignal.cancel();
                }
                Log.e(TAG, "返回HOME，所以取消操作 : ");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        finish();
    }

    public static boolean isServiceRunning(Context mContext,String className) {

        boolean isRunning = false;
        ActivityManager activityManager = (ActivityManager)
                                          mContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> serviceList
            = activityManager.getRunningServices(30);

        if (!(serviceList.size()>0)) {
            return false;
        }

        for (int i=0; i<serviceList.size(); i++) {
            if (serviceList.get(i).service.getClassName().equals(className) == true) {
                isRunning = true;
                break;
            }
        }
        return isRunning;
    }
}
