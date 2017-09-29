package com.btlfinger.fingerprintunlock.ui.support;

import java.util.ArrayList;

import android.R.integer;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.drawable.AnimationDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Vibrator;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.LinearLayout;
import android.widget.Toast;
import android.text.TextUtils;

import com.btlfinger.fingerprintunlock.AppData;
import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.aidlservice.FingerService;
import com.btlfinger.fingerprint.FingerPrintManager;
import com.btlfinger.fingerprint.FingerPrintModel;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
import com.btlfinger.fingerprintunlock.ui.custom.FingerProcess;
import com.btlfinger.fingerprintunlock.ui.custom.IndexOfFingerProcess;
import com.btlfinger.service.aidl.IFpsFingerClient;
import com.btlfinger.service.aidl.IFpsFingerManager;

//import android.app.FpsFingerManager;
//import android.app.IFpsFingerClient;
import android.os.RemoteException;

/**
 * 添加指纹界面
 * 
 * @author blestech
 * @since 2015-11-26
 */
public class AddFpActivity extends Activity implements OnClickListener {
	private static final String TAG = "AddFpActivity";
	private static final int RECORD_COUNT = 9;
	private FingerProcess mFingerProcessView = null;
	private IndexOfFingerProcess mIndexOfFingerProcess = null;
	private FingerPrintManager mFingerPrintManager = null;
	private EditText mNamEditText = null;
	private ImageView mGuideImageView = null;
	private static Button mContinue = null;
	private Button mRename = null;
	private Button mFinish = null;
	private static LinearLayout mEndLayout = null;
	private static int mCurrentStep = 0;

	private static String DEFAULT_FP_NAME = "new finger";
	private String mFpName = DEFAULT_FP_NAME;

	private static AppData mAppData = null;

	private static FingerPrintModel mFingerPrintModel = new FingerPrintModel();

	private int mFileIndex = 0;
	private AnimationDrawable anim;

	private boolean mHasCancelAction = false;

	private static int mFingerMaxIndex = 0;
    private static int mFpNameIndex = 0;
    
    private TextView mGuideTitle = null;
    private TextView mGuideContent = null;
    private TextView mGuideContentNext = null;
    
    private PowerManager mPowerManager = null;
	private WakeLock mWakeLock = null;
	private Vibrator vibrator = null;
	
    //private FpsFingerManager fm;
	private IFpsFingerManager fm;
    private IFpsFingerClient client = new IFpsFingerClient.Stub() {

		@Override
		public void getValue(int type, int score) throws RemoteException {
		
		    Log.i("client_AddFpActivity", "type = " + type + "---score = "
					+ score);
			if (type == 2) {// 注册时候的返回值
				if (score == 0) {// score:0 表示录入成功
					Log.i("client_AddFpActivity", "录入成功---下一步");
					fpHandler.sendEmptyMessage(REGISTER_SUCCESS);
				} else {
					Log.i("client_AddFpActivity", "录入失败---再次注册");
					fpHandler.sendEmptyMessage(REGISTER_FAIL);
				}
			} else if (type == /* FpsFingerManagerService.ISFINGERUP_WHAT */7) {// 判断手指是否抬起时的返回值
				if (score == -1) {// 表示手指未抬起
					Log.i("client_AddFpActivity", "手指没有抬起哦");
					fpHandler.postDelayed(fingerUpRunnable, 10);
				} else {
					fpHandler.sendEmptyMessage(REGISTER_AGAIN);
					Log.i("client_AddFpActivity", "手指抬起,再次注册");
				}
			}
		}
	};

	
	public void initService() {
		Intent intent = new Intent();
		intent.setAction("com.btlfinger.aidlservice");
		bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
	}

	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			// TODO Auto-generated method stub

			fm = IFpsFingerManager.Stub.asInterface(service);
			try {
				fm.listen(client);
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				fm.initRegister();
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}// 每次注册前需要初始化算法，必须的！！！
			try {
				fm.register();
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			// TODO Auto-generated method stub
			fm = null;
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		ActionBar actionBar = getActionBar();
		actionBar.setTitle(R.string.app_name);
		actionBar.setHomeButtonEnabled(false);
		actionBar.setDisplayShowTitleEnabled(true);
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setDisplayUseLogoEnabled(false);
		actionBar.setDisplayShowHomeEnabled(false);
		mHasCancelAction = false;

		// fm = (FpsFingerManager)
		// this.getSystemService(Context.FINGER_SERVICE);
		// fm.listen(client);
		initService();

		mFingerPrintManager = new FingerPrintManager(this);
		mFingerPrintModel = null;

		mAppData = (AppData) getApplication();

		mCurrentStep = 0;

		mFingerMaxIndex = mFingerPrintManager.getMaxId() + 1;
		mFpNameIndex = mFingerPrintManager.getNewFpNameIndex();
		mAppData.setFinger_count(mFingerMaxIndex);
		mAppData.setSafe_lock(FingerPrintManager.MSG_RES.MSG_REG_START);

		FingerPrintManager.init_folder();

		this.mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
		this.mWakeLock = this.mPowerManager.newWakeLock(
				PowerManager.FULL_WAKE_LOCK, "my lock");

		this.vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);

		setContentView(R.layout.activity_add_fp);

		mGuideTitle = (TextView) findViewById(R.id.id_add_guide_title);
		mGuideContent = (TextView) findViewById(R.id.id_add_guide_content);
		mGuideContentNext = (TextView) findViewById(R.id.id_add_guide_content_next);


		mContinue = (Button) findViewById(R.id.id_btn_stop);
		mRename = (Button) findViewById(R.id.id_btn_rename);
		mFinish = (Button) findViewById(R.id.id_btn_finish);
		mGuideImageView = (ImageView) findViewById(R.id.id_add_guide);
		mGuideImageView.setBackgroundResource(R.drawable.guide_anim);
		anim = (AnimationDrawable) mGuideImageView.getBackground();
		anim.stop();
		anim.start();

		mEndLayout = (LinearLayout) findViewById(R.id.id_ll_setup_end);

		mContinue.setOnClickListener(this);
		mRename.setOnClickListener(this);
		mFinish.setOnClickListener(this);

		mFingerProcessView = (FingerProcess) findViewById(R.id.id_fp_process);

		mIndexOfFingerProcess = (IndexOfFingerProcess) findViewById(R.id.id_fp_index_process);

		mIndexOfFingerProcess.setVisibility(View.GONE);
		mGuideTitle.setText(R.string.record_fp_title_positive);

		setCurrentStep(0);
		mFileIndex = mFingerMaxIndex;
	}

	public void setCurrentStep(int step) {
		mFingerProcessView.mCurrentStep = 0;
		mFingerProcessView.twikcleImg(this, step);
		mIndexOfFingerProcess.mCurrentStep = 0;
		mIndexOfFingerProcess.twikcleImg(this, step);
	}

	private static final int REGISTER_SUCCESS = 1;
	private static final int REGISTER_FAIL = 2;
	private static final int REGISTER_AGAIN = 3;

	Runnable fingerUpRunnable = new Runnable() {

		@Override
		public void run() {
			try {
				fm.isFingerUp();
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	};

	private Handler fpHandler = new Handler() {
		public void handleMessage(Message msg) {
			Log.e(TAG, "MSG : " + msg.what);
			if (mAppData.getSafe_lock() != FingerPrintManager.MSG_RES.MSG_REG_CANCEL) {

				if (msg.what == REGISTER_AGAIN) {
					Log.e(TAG, "第" + mCurrentStep + "步，设置引导内容！");
					mGuideContentNext
							.setText(R.string.record_fp_content_remove_first);
					try {
						fm.initRegister();
						fm.register();
					} catch (RemoteException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				} else if (msg.what == REGISTER_SUCCESS) {

					if (vibrator != null) {
						vibrator.vibrate(100);
					}

					if (mCurrentStep == RECORD_COUNT) {

						Log.e(TAG, "第" + mCurrentStep + "步，录取完成拉！ ");

						// fm.saveData();/*第10步完成，通知服务保存数据*/

						fpHandler.removeCallbacks(fingerUpRunnable);
						mCurrentStep++;

						mGuideTitle.setText(R.string.setup_end);
						mGuideContentNext.setText(R.string.record_successfully);
						setCurrentStep(mCurrentStep);

						mFileIndex = mFileIndex + 1;
						try {
							chmodFile(mFingerMaxIndex);
							mAppData.chmodDir();
						} catch (Exception e) {
							// TODO: handle exception
						}

						mEndLayout.setVisibility(View.VISIBLE);
						mContinue.setVisibility(View.GONE);
					} else if (mCurrentStep < RECORD_COUNT) {

						Log.e(TAG, "第" + mCurrentStep + "步，设置引导内容！");
						mGuideContentNext
								.setText(R.string.record_fp_content_remove);
						showGuideContentAnim(mGuideContentNext, mGuideContent);

						mGuideImageView.setVisibility(View.GONE);
						mFingerProcessView.setVisibility(View.VISIBLE);

						mCurrentStep++;

						Log.e(TAG, "第" + mCurrentStep + "步，继续录入 ");
						setCurrentStep(mCurrentStep);
						mFileIndex = mFileIndex + 1;

						try {
							chmodFile(mFingerMaxIndex);
							mAppData.chmodDir();
						} catch (Exception e) {
							// TODO: handle exception
						}

						// fm.register();
						// 检测手指有没有抬起
						fpHandler.post(fingerUpRunnable);
					}
				} else if (msg.what == REGISTER_FAIL) {
					try {
						fm.initRegister();
						fm.register();
					} catch (RemoteException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}

		}
	};

	public void showGuideContentAnim(TextView enterView, TextView exitView) {
		Animation exitanimation = AnimationUtils.loadAnimation(this,
				R.anim.register_title_text_exit);
		Animation enteranimation = AnimationUtils.loadAnimation(this,
				R.anim.register_title_text_enter);
		exitanimation.setAnimationListener(new TitleExitAnimListener(exitView,
				View.GONE));
		enteranimation.setAnimationListener(new TitleExitAnimListener(
				enterView, View.VISIBLE));
		exitView.startAnimation(exitanimation);
		enterView.startAnimation(enteranimation);
	}

	private class TitleExitAnimListener implements AnimationListener {
		TextView mView;
		int mVisible;

		public TitleExitAnimListener(TextView view, int visible) {
			mView = view;
			mVisible = visible;
		}

		@Override
		public void onAnimationEnd(Animation arg0) {

			if (null != mView) {
				mView.setVisibility(mVisible);
			}
		}

		@Override
		public void onAnimationRepeat(Animation arg0) {
		}

		@Override
		public void onAnimationStart(Animation arg0) {
		}
	}

	Toast mToast = null;

	private void showToast(boolean isFail) {
		String text = getString(R.string.new_name_warning);

		if (!isFail) {
			text = getString(R.string.new_name_warning);
		} else {
			text = getString(R.string.fp_addfail);
		}
		if (mToast == null) {
			mToast = Toast.makeText(AddFpActivity.this, text,
					Toast.LENGTH_SHORT);
		} else {
			mToast.setText(text);
			mToast.setDuration(Toast.LENGTH_SHORT);
		}
		mToast.show();
	}

	public void chmodFile(int fileindex) {
		String filenameString = FingerPrintManager.FP_DATA_DIR + fileindex;

		String command = "chmod 777 " + filenameString;// 全部权限
		Runtime runtime = Runtime.getRuntime();
		try {
			Process proc = runtime.exec(command);
		} catch (Exception e) {
			// TODO: handle exception
		}

	}

	public void addFingerModelToList(int templateSize) {
		FingerPrintModel finger = new FingerPrintModel();
		finger.fp_data_index = mFingerMaxIndex;
		finger.fp_template_size = templateSize;
		finger.fp_name = DEFAULT_FP_NAME + mFpNameIndex;
		finger.fp_data_path = mFingerMaxIndex;
		mFingerPrintModel = finger;
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	@Override
	protected void onResume() {
		super.onResume();
		this.mWakeLock.acquire();
	}

	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		unbindService(mConnection);
		super.onStop();
	}

	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
		mAppData.setSafe_lock(FingerPrintManager.MSG_RES.MSG_REG_CANCEL);
		if (mHasCancelAction != true) {
			try {
				fm.cancle(1);
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			setResult(-302);
		}
		Log.e(TAG, "onPause +++ ");
		try {
			fm.waitScreenOn();
		} catch (RemoteException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		Log.e(TAG, "onPause --- ");
		this.mWakeLock.release();// 取消屏幕常亮
		finish();
	}
	
	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		// 取消录取
		case R.id.id_btn_stop:

			fpHandler.removeCallbacks(fingerUpRunnable);
			mAppData.setSafe_lock(FingerPrintManager.MSG_RES.MSG_REG_CANCEL);
			//PreferenceUtils.enablePwd(false);
			setResult(-302);

			mHasCancelAction = true;
			
			try {
				fm.cancle(1);
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			finish();
			break;
		case R.id.id_btn_rename:
			createRenameDialog();
			break;
		case R.id.id_btn_finish:
			mHasCancelAction = true;
			try {
				fm.saveData();
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}// 必须点击完成之后再通知服务保存数据
			//PreferenceUtils.enablePwd(false);
			setResult(FingerPrintManager.MSG_RES.RES_ADD_CODE);
			finish();
			//跳转到Setting_SECURITY安全界面
			break;
		default:
			break;
		}

	}
	/**
	 * 通过Intent对象开启系统选择解锁方式界面
	 */
	private void gotoManageS() {
		Intent intent = new Intent(Intent.ACTION_MAIN);
		intent.addCategory(Intent.CATEGORY_LAUNCHER);            
		ComponentName cn = new ComponentName("com.android.settings", "com.android.settings.ChooseLockGeneric");            
		intent.setComponent(cn);
		startActivity(intent);
		
	}

	private void createRenameDialog() {
		mNamEditText = new EditText(this);
		mNamEditText.setBackgroundResource(R.drawable.edittext_bg);
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		mNamEditText.setText(DEFAULT_FP_NAME + mFpNameIndex);
		mNamEditText.setGravity(Gravity.CENTER_HORIZONTAL);
		mNamEditText.setPadding(15, 15, 15, 15);
		builder.setTitle(R.string.fp_name).setView(mNamEditText)
				.setPositiveButton(R.string.confirm, confirmListener)
				.setNegativeButton(R.string.cancel, null).create().show();
	}

	private DialogInterface.OnClickListener confirmListener = new DialogInterface.OnClickListener() {
		@Override
		public void onClick(DialogInterface dialog, int which) {
			String newName = mNamEditText.getText().toString().trim();
			/* 判断输入的重命名是否为空 */
			if (!TextUtils.isEmpty(newName)) {

				try {
					fm.saveDataWithName(newName);
				} catch (RemoteException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}// 请求服务保存数据
				//PreferenceUtils.enablePwd(false);
				setResult(FingerPrintManager.MSG_RES.RES_ADD_CODE);
				finish();
			} else {
				showToast(false);
			}
		}
	};

	public void renameFp(String editname) {
		mFingerPrintModel.fp_name = editname;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			setResult(111);
			//PreferenceUtils.enablePwd(false);
		}
		return super.onKeyDown(keyCode, event);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem menuItem) {
		switch (menuItem.getItemId()) {

		case android.R.id.home:
			//PreferenceUtils.enablePwd(false);
			finish();
			break;

		default:
			break;
		}

		return true;
	}

}
