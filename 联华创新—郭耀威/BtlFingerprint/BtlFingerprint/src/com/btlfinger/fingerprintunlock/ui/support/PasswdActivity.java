package com.btlfinger.fingerprintunlock.ui.support;
import android.app.ActionBar;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.view.KeyEvent;
import android.util.Log;
//import com.cdfinger.fingerprintunlock.PackagesConstant;
import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.fingerprintunlock.applock.EnableLockAppActivity;
import com.btlfinger.fingerprintunlock.applock.NumberPad;
import com.btlfinger.fingerprintunlock.applock.ProcessIndication;
import com.btlfinger.fingerprintunlock.applock.NumberPad.OnNumberClickListener;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
//import com.android.internal.widget.LockPatternUtils;
import android.app.admin.DevicePolicyManager;

/**
 * 安全密码界面
 * 
 * @author cdfinger
 * @since 2015-2-4
 */
public class PasswdActivity extends Activity implements OnNumberClickListener {

    private static final String TAG = "PasswdActivity";
	private TextView mTextView = null;
	private int mIsSafety = 0;
	private boolean mFlag = false;
	private StringBuilder mPassword = null;
	private ProcessIndication mIndication = null;
	private NumberPad mNumberPad = null;
	private TextView mTitleTextView = null;
	//private LockPatternUtils mLockPatternUtils;/*yeyunfeng*/
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_passwd);

        //mLockPatternUtils = new LockPatternUtils(this);/*yeyunfeng*/

		ActionBar.LayoutParams lp = new ActionBar.LayoutParams(
        ActionBar.LayoutParams.MATCH_PARENT,
        ActionBar.LayoutParams.MATCH_PARENT, Gravity.CENTER);

		View viewTitleBar = getLayoutInflater().inflate( R.layout.pwd_actionbar_layout, null);
		ActionBar actionBar = getActionBar();
		actionBar.setCustomView(viewTitleBar, lp);
		actionBar.setDisplayShowHomeEnabled(false);
		actionBar.setDisplayShowTitleEnabled(false);
		actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM);
		actionBar.setDisplayShowCustomEnabled(true);

		mIndication = (ProcessIndication) findViewById(R.id.id_lockui_indication);
		mIndication.mCircleColor = Color.BLACK;
		mNumberPad = (NumberPad) findViewById(R.id.id_lockui_numberpad);
		mNumberPad.setOnNumberClickListener(this);
		mPassword = new StringBuilder();
		mIsSafety = PreferenceUtils.isSafety();

		mTextView = (TextView) findViewById(R.id.id_tv_input_pwd);
		mTitleTextView = (TextView) findViewById(R.id.id_tv_input_pwd_title);
		
		/*if(!PreferenceUtils.isNeedPwd()){
			mTextView.setText(getString(R.string.input_pwd));
			mFlag = true;
		}	*/
		
		/*if (mIsSafety == 0) {
			//未存在密码，要求新建密码
			mTitleTextView.setText(R.string.input_pwd_title_new);
		} else if (mIsSafety == 1){ 
			//已经存在密码，要求更新密码
			mTitleTextView.setText(R.string.input_pwd_title_update);		
	    } else {
			//已经存在密码，要求输入密码
			mTextView.setText(getString(R.string.input_pwd));	
		}*/
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		/*if (keyCode == KeyEvent.KEYCODE_BACK && mIsSafety != 1) {
		    Intent home = new Intent(Intent.ACTION_MAIN);  
            home.addCategory(Intent.CATEGORY_HOME);   
            startActivity(home);
            finish();
			return true;
		} else if (keyCode == KeyEvent.KEYCODE_BACK && mIsSafety == 1){
		    PreferenceUtils.setSafety(2);
    	    Log.e(TAG, "PasswdActivity onKeyDown 设置无需密码");
    	    PreferenceUtils.enablePwd(false);
		    setResult(111);
		    return super.onKeyDown(keyCode, event);
		}*/
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			if(mIsSafety == 1){
				Intent intent = new Intent(this, EnableLockAppActivity.class);
				startActivity(intent);
			}
			
			finish();
			return true;
		} 
		
		return super.onKeyDown(keyCode, event);
	}
	
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		/*if (!PreferenceUtils.getPwd().equals(PreferenceUtils.VLUE_ERROR_PWD)) {
		    PreferenceUtils.setSafety(2);
		}*/
	    finish();	
	}
	
	@Override
    protected void onPause() {
        super.onPause();
        /*if(mFlag){
        	PreferenceUtils.enablePwd(false);
        }else{
        	PreferenceUtils.enablePwd(true);
        }*/
        finish();
    }
	/*@Override
	public boolean onOptionsItemSelected(MenuItem menuItem) {
		switch (menuItem.getItemId()) {
		
		case android.R.id.home:
			setResult(111);
		    Log.e(TAG, "FpDetail onDestroy 设置无需密码");
		    PreferenceUtils.enablePwd(false);
			finish();
			break;
			
		default:
			break;
		}
		
		return true;
	}*/
	
	public void goToManagerFp() {
		Intent intent = new Intent();
		intent.setClass(this, ManageFpActivity.class);
		startActivity(intent);
	}

	Toast mToast = null;
	/**
	 * 显示剩余时间
	 */
	public void showLeftTime() {
        if (mToast == null) {
            mToast = Toast.makeText(PasswdActivity.this, "请"+(30-mWaitTime)+"秒后再试", Toast.LENGTH_SHORT);
        } else {
            mToast.setText("请"+(30-mWaitTime)+"秒后再试");  
            mToast.setDuration(Toast.LENGTH_SHORT);
        }
        mToast.show();
	}
	
	private void updateLockPass() {
	    String existPass = PreferenceUtils.getPwd();
        if (existPass != null && existPass != PreferenceUtils.VLUE_ERROR_PWD) {
            boolean isFallback = false;
            int mRequestedQuality = DevicePolicyManager.PASSWORD_QUALITY_NUMERIC;
            //mLockPatternUtils.clearLock(isFallback);
            //mLockPatternUtils.saveLockPassword(existPass, mRequestedQuality, isFallback);
        }
	}

	private int mEditPwdCount = 0;//编辑（创建或更新）密码的次数
	private int mInputPwdCount = 0;//输入密码的次数，超过5次则30秒内禁止输入密码
	private boolean mNberPadClickable = true;
	private int mWaitTime = 0;//禁止输入密码后等待的时间
	private String mFirstPwd = null;
	private Handler mPwdHandler = new Handler();
	
	@Override
	public void clickNumer(int number) {
        Log.e(TAG, "number : " + number );
        if (number == -2) {
            /*if ( mIsSafety == 1 ) {	//点击消键
                //如果是更新密码，则直接取消操作
                PreferenceUtils.setSafety(2);
                PreferenceUtils.enablePwd(false);
            } else {
                //如果是新建密码或者要求输入密码，取消要返回HOME
                Intent home = new Intent(Intent.ACTION_MAIN);  
                home.addCategory(Intent.CATEGORY_HOME);   
                startActivity(home); 
            }
            finish();*/
        } else if (number == -1) {// 如果点击删除键
            if (mPassword.length() > 0) {
                mPassword.deleteCharAt(mPassword.length() - 1);
            }
		} else {
			if (mNberPadClickable == true) {
				// 如果是其他数字按键
				mPassword.append(number);
				String pwd = mPassword.toString();
				
				if(pwd.length() == 4 ){
					mPassword.delete(0, mPassword.length());
					if(mFlag){
						if (mEditPwdCount == 0) {
							mFirstPwd = pwd;			
							mTextView.setText(getString(R.string.input_confirm_pwd));
							mIndication.setIndex(0);
							mEditPwdCount +=1;
						} else if (mEditPwdCount == 1){
							if (mFirstPwd != null && pwd.equals(mFirstPwd)) {
								PreferenceUtils.putPwd(pwd);
								/*if (PreferenceUtils.isSettingFpScreenLockOn()) {
								    updateLockPass();//更新密码解锁密码
								}*/
								mFirstPwd = null;
								//PreferenceUtils.setSafety(2);
								//Log.e(TAG, "PasswdActivity设置无需密码:"+PreferenceUtils.isNeedPwd());
								//PreferenceUtils.enablePwd(true);
								//mFlag = false;
								//goToManagerFp();
								Intent intent = new Intent(this, EnableLockAppActivity.class);
						    	startActivity(intent);
								finish();
							} else {
								mEditPwdCount = 0;
								mIndication.setIndex(0);
								mTextView.setText(getString(R.string.input_confirm_pwd_error));
							}
						}
					}else{
						Log.e(TAG, pwd+"|pwd:"+PreferenceUtils.getPwd());
						if(pwd.equals(PreferenceUtils.getPwd())){
							if(mIsSafety == 1){
								mFlag = true;
								mTextView.setText(getString(R.string.input_pwd));
							}else{
								Intent intent = new Intent(this, EnableLockAppActivity.class);
						    	startActivity(intent);
						    	finish();
							}
						}else{
							mInputPwdCount += 1;
							Animation shake = AnimationUtils.loadAnimation(
									PasswdActivity.this, R.anim.shake);// 加载动画资源文件
							mIndication.startAnimation(shake); // 给组件播放动画效果
							
							if (mInputPwdCount == 5) {
								mNberPadClickable = false;
								mPwdHandler.post(new Runnable() {
									
									@Override
									public void run() {
										mPwdHandler.postDelayed(this, 1000);
										mWaitTime+=1;
										Log.e(TAG,"mWaitTime = "+mWaitTime);
										if (mWaitTime == 30) {
											mWaitTime = 0;
											mInputPwdCount = 0;
											mPwdHandler.removeCallbacks(this);
											mNberPadClickable = true;
										}
									}
								});
							}
						}
					}		
				}

				/*if(!PreferenceUtils.isNeedPwd()){
					if (pwd.length() == 4) {
						if (mEditPwdCount == 0) {
							mFirstPwd = pwd;			
							mTextView.setText(getString(R.string.input_confirm_pwd));
							mIndication.setIndex(0);
							mPassword.delete(0, mPassword.length());
							mEditPwdCount +=1;
						} else if (mEditPwdCount == 1){
							if (mFirstPwd != null && pwd.equals(mFirstPwd)) {
								PreferenceUtils.putPwd(pwd);
								if (PreferenceUtils.isSettingFpScreenLockOn()) {
								    updateLockPass();//更新密码解锁密码
								}
								mFirstPwd = null;
								//PreferenceUtils.setSafety(2);
								Log.e(TAG, "PasswdActivity设置无需密码:"+PreferenceUtils.isNeedPwd());
								PreferenceUtils.enablePwd(true);
								mFlag = false;
								//goToManagerFp();
								finish();
							} else {
								mEditPwdCount = 0;
								mIndication.setIndex(0);
								mPassword.delete(0, mPassword.length());
								mTextView.setText(getString(R.string.input_confirm_pwd_error));
							}
						}
					}
				}else{
					if (pwd.equals(PreferenceUtils.getPwd())) {
						mPassword.delete(0, mPassword.length());
						
						Log.e(TAG, "PasswdActivity设置无需密码1111:"+PreferenceUtils.isNeedPwd());
						//PreferenceUtils.enablePwd(false);
						if(mIsSafety == 1){
							//goToManagerFp();
							PreferenceUtils.enablePwd(false);
							//mTextView.setText(getString(R.string.input_pwd));
						}else{
							Intent intent = new Intent(this, EnableLockAppActivity.class);
					    	startActivity(intent);
					    	finish();
						}
					} else if (pwd.length() == 4) {
						mInputPwdCount += 1;
						Animation shake = AnimationUtils.loadAnimation(
								PasswdActivity.this, R.anim.shake);// 加载动画资源文件
						mIndication.startAnimation(shake); // 给组件播放动画效果
						mPassword.delete(0, mPassword.length());
						
						if (mInputPwdCount == 5) {
							mNberPadClickable = false;
							mPwdHandler.post(new Runnable() {
								
								@Override
								public void run() {
									mPwdHandler.postDelayed(this, 1000);
									mWaitTime+=1;
									Log.e(TAG,"mWaitTime = "+mWaitTime);
									if (mWaitTime == 30) {
										mWaitTime = 0;
										mInputPwdCount = 0;
										mPwdHandler.removeCallbacks(this);
										mNberPadClickable = true;
									}
								}
							});
						}
					}	
				}*/

				/*if (mIsSafety == 0 || mIsSafety == 1) {//如果是新建密码或者修改密码
					if (pwd.length() == 4) {			
						if (mEditPwdCount == 0) {
							mFirstPwd = pwd;
							
							mTextView.setText(getString(R.string.input_confirm_pwd));
							mIndication.setIndex(0);
							mPassword.delete(0, mPassword.length());
							mEditPwdCount +=1;
						} else if (mEditPwdCount == 1){
							if (mFirstPwd != null && pwd.equals(mFirstPwd)) {
								PreferenceUtils.putPwd(pwd);
								if (PreferenceUtils.isSettingFpScreenLockOn()) {
								    updateLockPass();//更新密码解锁密码
								}
								mFirstPwd = null;
								PreferenceUtils.setSafety(2);
								Log.e(TAG, "PasswdActivity设置无需密码");
								PreferenceUtils.enablePwd(false);
								goToManagerFp();
								finish();
							} else {
								mIndication.setIndex(0);
								mPassword.delete(0, mPassword.length());
								mTextView.setText(getString(R.string.input_confirm_pwd_error));
							}
						}
					}
				} else {
					if (pwd.equals(PreferenceUtils.getPwd())) {
						mPassword.delete(0, mPassword.length());
						
						Log.e(TAG, "PasswdActivity设置无需密码");
						PreferenceUtils.enablePwd(false);
						goToManagerFp();
						finish();

					} else if (pwd.length() == 4) {
						mInputPwdCount += 1;
						Animation shake = AnimationUtils.loadAnimation(
								PasswdActivity.this, R.anim.shake);// 加载动画资源文件
						mIndication.startAnimation(shake); // 给组件播放动画效果
						mPassword.delete(0, mPassword.length());
						
						if (mInputPwdCount == 5) {
							mNberPadClickable = false;
							mPwdHandler.post(new Runnable() {
								
								@Override
								public void run() {
									mPwdHandler.postDelayed(this, 1000);
									mWaitTime+=1;
									Log.e(TAG,"mWaitTime = "+mWaitTime);
									if (mWaitTime == 30) {
										mWaitTime = 0;
										mInputPwdCount = 0;
										mPwdHandler.removeCallbacks(this);
										mNberPadClickable = true;
									}
								}
							});
						}
					}
				}*/
			} else {
				showLeftTime();
		    }
		} 
		mIndication.setIndex(mPassword.length());
	}
}
