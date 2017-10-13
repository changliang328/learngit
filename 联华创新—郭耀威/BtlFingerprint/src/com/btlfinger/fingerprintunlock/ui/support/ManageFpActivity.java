package com.btlfinger.fingerprintunlock.ui.support;

import java.util.ArrayList;

import java.util.List;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AbsListView.LayoutParams;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Button;
import android.widget.Toast;
import android.widget.ToggleButton;
import android.widget.LinearLayout;
import android.hardware.fingerprint.Fingerprint;
import android.hardware.fingerprint.FingerprintManager;

import com.btlfinger.fingerprintunlock.PackagesConstant;
import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.fingerprint.FingerPrintManager;
import com.btlfinger.fingerprint.dao.FpsTable;
import com.btlfinger.fingerprint.utils.CrashHandlerUtils;
import com.btlfinger.fingerprint.FingerPrintModel;
import com.btlfinger.fingerprintunlock.applock.EnableLockAppActivity;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
import android.net.Uri;
import android.app.admin.DevicePolicyManager;
import com.android.internal.widget.LockPatternUtils;


import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;


/**
 * 指纹主界面
 *
 * @author blestech
 * @since 2015-11-26
 */
public class ManageFpActivity extends Activity {
    private String TAG = "ManageFpActivity";
    private static Context mContext = null;
    private boolean mNeedPasswd = true;
    private ToggleButton pToggleButton = null;
    private ToggleButton mToggleButton = null;
  //  private ToggleButton rToggleButton = null; 
    private ToggleButton aToggleButton = null;
    private ToggleButton nToggleButton = null;			//davie
    private LinearLayout xFpManagerLayout = null;
    private TextView     tFpManagerTv = null;
    private LockPatternUtils mLockPatternUtils;
    private TextView tFpVersionNum = null;
    
    private final String SYS_PATH = "/sys/bl229x_sysfs/keycode";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_manage_fp);

        ActionBar.LayoutParams lp = new ActionBar.LayoutParams(
            ActionBar.LayoutParams.MATCH_PARENT,
            ActionBar.LayoutParams.MATCH_PARENT, Gravity.CENTER);
        View viewTitleBar = getLayoutInflater().inflate( R.layout.actionbar_layout, null);
        //设置action
        ActionBar actionBar = getActionBar();
        actionBar.setCustomView(viewTitleBar, lp);
        actionBar.setDisplayShowHomeEnabled(false);
        actionBar.setDisplayShowTitleEnabled(false);
        actionBar.setDisplayShowCustomEnabled(true);


        mContext = this;

        pToggleButton = (ToggleButton) findViewById(R.id.photo_control_tb);
        mToggleButton = (ToggleButton) findViewById(R.id.music_cntrol_tb);
        //rToggleButton = (ToggleButton) findViewById(R.id.reading_control_tb);
        aToggleButton = (ToggleButton) findViewById(R.id.answer_control_tb);
        nToggleButton = (ToggleButton) findViewById(R.id.navgation_func_tb);	//davie
        //pToggleButton.setOnCheckedChangeListener(fingerprintCheckedChangeListener);
        //mToggleButton.setOnCheckedChangeListener(fingerprintCheckedChangeListener);
        //rToggleButton.setOnCheckedChangeListener(fingerprintCheckedChangeListener);
       // aToggleButton.setOnCheckedChangeListener(fingerprintCheckedChangeListener);

        /*LinearLayout containerMusic = (LinearLayout)findViewById(R.id.music_tb);
        containerMusic.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				Log.e(TAG,
						"发生变化 ： containerMusic.setOnClickListener"
								+ mToggleButton.isChecked());
				
				boolean isChecked = !mToggleButton.isChecked();
				mToggleButton.setChecked(isChecked);
				PreferenceUtils.setSettingFpMusicLockOffOn(isChecked);
			}
		});	
        
        LinearLayout containerCamera = (LinearLayout)findViewById(R.id.camera_tb);
        containerCamera.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				Log.e(TAG,
						"发生变化 ： containerCamera.setOnClickListener"
								+ pToggleButton.isChecked());
				
				boolean isChecked = !pToggleButton.isChecked();
				pToggleButton.setChecked(isChecked);
				PreferenceUtils.setSettingFpCameraLockOffOn(isChecked);
			}
		});	
        
        LinearLayout containerPhone = (LinearLayout)findViewById(R.id.phone_tb);
        containerPhone.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				Log.e(TAG,
						"发生变化 ： containerPhone.setOnClickListener"
								+ aToggleButton.isChecked());
				
				boolean isChecked = !aToggleButton.isChecked();
				aToggleButton.setChecked(isChecked);
				PreferenceUtils.setSettingFpCallLockOffOn(isChecked);
			}
		});	*/
        mLockPatternUtils = new LockPatternUtils(this);
        initToggleButton();
        
        tFpVersionNum = (TextView)findViewById(R.id.num_manage_version);
        tFpVersionNum.setText(getString(R.string.application_version) +"V2.0");
        
        TextView mTextView = (TextView) findViewById(R.id.manage_applock_pwd);
		String pwdString = Settings.System.getString(getContentResolver(),PackagesConstant.SETTINGS_INITIALIZE_PWD);		
		if(null == pwdString){
			pwdString = "0000";
		}
		
		if(PreferenceUtils.VLUE_ERROR_PWD == PreferenceUtils.getPwd()){
			PreferenceUtils.putPwd(pwdString);
		}

		mTextView.setText(getString(R.string.fingerprint_password_note) + pwdString);
    }



    @Override
    protected void onResume() {
        Log.e(TAG, "ManageFpActivity -->onResume");
        super.onResume();
    }
    @Override
    protected void onPause() {
        super.onPause();
    }

    /**
     * 初始化开关
     */
    public void initToggleButton() {
        Log.e(TAG, TAG + "初始化 ： initToggleButton()");
        Log.e(TAG, "initToggleButton:"+ PreferenceUtils.isSettingFpCameraLockOn());
        pToggleButton.setChecked(PreferenceUtils.isSettingFpCameraLockOn());
        mToggleButton.setChecked(PreferenceUtils.isSettingFpMusicLockOn());
       // rToggleButton.setChecked(PreferenceUtils.isSettingFpReadingPageOffOn());
        aToggleButton.setChecked(PreferenceUtils.isSettingFpCallLockOn());
        nToggleButton.setChecked(PreferenceUtils.isSettingFpNavFuncOn());
    }

    /**
     * 设置监听
     */
    private OnCheckedChangeListener fingerprintCheckedChangeListener = new OnCheckedChangeListener() {

        @Override
        public void onCheckedChanged(CompoundButton buttonView,
                                     boolean isChecked) {
            switch (buttonView.getId()) {
				
            case R.id.photo_control_tb:
                Log.i(TAG, "onCheckedChanged:"+pToggleButton.isChecked());
                PreferenceUtils.setSettingFpCameraLockOffOn(pToggleButton.isChecked());
                break;
				
            case R.id.music_cntrol_tb:
                Log.i(TAG, "onCheckedChanged:"+mToggleButton.isChecked());
                PreferenceUtils.setSettingFpMusicLockOffOn(mToggleButton.isChecked());
                break;
          
            case R.id.answer_control_tb:
                Log.i(TAG, "onCheckedChanged:"+aToggleButton.isChecked());
                PreferenceUtils.setSettingFpCallLockOffOn(aToggleButton.isChecked());
                break;
            }

        }

    };
    
    /**
     * 开启EnableLockAppActivity界面
     * @param view
     */
    public void gotoLockApps(View view) {
    	PreferenceUtils.setSafety(2);
		//PreferenceUtils.enablePwd(false);
		Intent intent = new Intent();
		intent.setClass(this, PasswdActivity.class);
		//intent.setFlags(1);
		startActivity(intent);
    	//Intent intent = new Intent(this, EnableLockAppActivity.class);
    	//startActivity(intent);
    }

    /**
     * 开启PasswdActivity界面
     *
     * @param view
     */
    public void gotoManageM(View view) {
		
		FingerprintManager fpm = (FingerprintManager)getSystemService(
				   Context.FINGERPRINT_SERVICE);
		if (!fpm.isHardwareDetected()) {
			   Log.v(TAG, "No fingerprint hardware detected!!");
			   return;
		}
		Intent intent = new Intent();
		final List<Fingerprint> items = fpm.getEnrolledFingerprints();
		final int fingerprintCount = items != null ? items.size() : 0;
		final String clazz;
		
		if (fingerprintCount > 0) {
			clazz = "com.android.settings.fingerprint.FingerprintSettings";
		} else {
			clazz = "com.android.settings.fingerprint.FingerprintEnrollIntroduction";
		}
		intent.setClassName("com.android.settings", clazz);
		
		startActivity(intent);
    }
	
    public void gotoManageMussic(View view) {
    	Log.i(TAG, "mToggleButton:11111111111:"+mToggleButton.isChecked());
    	boolean isChecked = !mToggleButton.isChecked();
		mToggleButton.setChecked(isChecked);
		PreferenceUtils.setSettingFpMusicLockOffOn(isChecked);
    }
    
    public void gotoManageCamera(View view) {
    	Log.i(TAG, "pToggleButton:11111111111:"+pToggleButton.isChecked());
    	boolean isChecked = !pToggleButton.isChecked();
		pToggleButton.setChecked(isChecked);
		PreferenceUtils.setSettingFpCameraLockOffOn(isChecked);
    }
    
    public void gotoManagePhone(View view) {
    	Log.i(TAG, "aToggleButton:11111111111:"+aToggleButton.isChecked());
    	boolean isChecked = !aToggleButton.isChecked();
		aToggleButton.setChecked(isChecked);
		PreferenceUtils.setSettingFpCallLockOffOn(isChecked);
    }
    public void gotoManageNav(View view) {			//davie
    	Log.i(TAG, "nToggleButton:11111111111:"+nToggleButton.isChecked());
    	boolean isChecked = !nToggleButton.isChecked();
		nToggleButton.setChecked(isChecked);
		PreferenceUtils.gotoSetNavFunc(isChecked);
		PreferenceUtils.setSettingFpNavFuncOffOn(isChecked);
    }
    /**
     * 开启ManageFpVersion界面
     * @param view
     */
    public void gotoManageV(View view) {
        Intent intent = new Intent(this, ManageFpVersion.class);
        startActivity(intent);
    }
    /**
     * 设置title返回home界面
     */
    public boolean onOptionsItemSelected(MenuItem menuItem) {
        switch (menuItem.getItemId()) {
        case android.R.id.home:
            finish();
            break;
        default:
            break;
        }
        return true;
    }


}
