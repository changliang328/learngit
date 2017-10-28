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
import android.provider.Settings.SettingNotFoundException;
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
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.ToggleButton;



import com.btlfinger.fingerprintunlock.PackagesConstant;
import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.fingerprint.FingerPrintManager;
import com.btlfinger.fingerprint.dao.FpsTable;
import com.btlfinger.fingerprint.FingerPrintModel;
import com.btlfinger.fingerprintunlock.applock.EnableLockAppActivity;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
import android.net.Uri;
import android.app.admin.DevicePolicyManager;
/**
 * 指纹管理界面
 * 
 * @author blestech
 * @since 2015-11-26
 */
public class ManageFmActivity extends Activity {
	private String TAG = "ManageFmActivity";
	private static Context mContext = null;
	private List<FingerPrintModel> mFingerPrintModels = new ArrayList<FingerPrintModel>();
	private ListView mListView = null;
	private LinearLayout mLinearLayout;
	private SimpleCursorAdapter mCursorAdapter = null;
	private Cursor mCursor = null;
	private TextView mAppLockTv = null;
	private TextView mAddFpTv = null;
	private boolean mNeedPasswd = true;
	private boolean mStopFlag = true;
	private ToggleButton mToggleButton = null;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mNeedPasswd = true;
		//PreferenceUtils.enablePwd(false);
		setContentView(R.layout.activity_manage_fm);
		ActionBar actionBar = getActionBar();
		actionBar.setTitle(R.string.app_name);
		actionBar.setHomeButtonEnabled(false);
		actionBar.setDisplayShowTitleEnabled(true); 
		actionBar.setDisplayShowHomeEnabled(false);
		actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayUseLogoEnabled(false);
		mContext = this;
		
		mListView = (ListView)findViewById(R.id.id_lv_finger);
		mAddFpTv = (TextView)findViewById(R.id.id_manage_addfp_tv);
		mLinearLayout = (LinearLayout) findViewById(R.id.lv_ll);
		mToggleButton = (ToggleButton) findViewById(R.id.manage_appunlock_tb);
		//mToggleButton.setOnCheckedChangeListener(fingerprintCheckedChangeListener);
		mListView.setAdapter(mCursorAdapter);
		mListView.setOnItemClickListener(mOnItemClickListener);
		updateListView();
		initToggleButton();

	}

	
	/**
	 * 设置可编辑
	 */
	protected void onResume() {
		Log.e(TAG, "ManageFmActivity -->onResume");
		super.onResume();
		
		updateListView();
		initToggleButton();
		mStopFlag = true;
	}
	
	@Override
	protected void onStop() {
		super.onStop();
		Log.e(TAG, "onStop: "+mStopFlag );
		if(mStopFlag){
			finish();
		}
	}
	
	/**
	 * 更新lock
	 */
	private void updateLockPass() {
	    String existPass = PreferenceUtils.getPwd();
        if (existPass != null && existPass != PreferenceUtils.VLUE_ERROR_PWD) {
            boolean isFallback = false;
            int mRequestedQuality = DevicePolicyManager.PASSWORD_QUALITY_NUMERIC;
        }
	}
	/**
	 * 初始化开关
	 */
	public void initToggleButton() {
		Log.e("TAG", TAG + "初始化 ： initToggleButton()" +PreferenceUtils.isSettingFpScreenLockOn());
		mToggleButton.setChecked(PreferenceUtils.isSettingFpScreenLockOn());
	}
	
	/**
	 * 设置监听
	 */
	private OnCheckedChangeListener fingerprintCheckedChangeListener = new OnCheckedChangeListener() {

		@Override
		public void onCheckedChanged(CompoundButton buttonView,
				boolean isChecked) {
			if(isChecked)
			{
				if (mFingerPrintModels != null && mFingerPrintModels.size() == 0) {
					addFingerPrint(buttonView.getId());
				}
			}

			PreferenceUtils.setSettingFpScreenLockOffOn(mToggleButton.isChecked());
			Log.i(TAG, "fingerprintCheckedChangeListener" + "mToggleButton.isChecked" + mToggleButton.isChecked()
					+ "setSettingFpScreenLockOffOn" +PreferenceUtils.isSettingFpScreenLockOn());
		}
	};
	
	
	private Handler mManageFpHandler = new Handler();
	/**
	 * handlder 管理屏幕锁
	 */
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {

		super.onActivityResult(requestCode, resultCode, data);

		Log.e(TAG, "onActivityResult---requestCode:" + requestCode + "---resultCode:" + resultCode);
		if (requestCode == FingerPrintManager.MSG_RES.REQ_ADD_CODE && resultCode == FingerPrintManager.MSG_RES.RES_ADD_CODE) {
			updateListView();
		} else if (requestCode == FingerPrintManager.MSG_RES.REQ_DETAIL_CODE
				&& (resultCode == FingerPrintManager.MSG_RES.RES_DELETE_CODE 
				|| resultCode == FingerPrintManager.MSG_RES.RES_RENAME_CODE)) {
			updateListView();
		} else if (requestCode == R.id.id_manage_applock_tv) {
            if (resultCode == FingerPrintManager.MSG_RES.RES_ADD_CODE) {
				updateListView();
				Log.e(TAG, "onActivityResult -->nablelockapps");
				
				mManageFpHandler.postDelayed(new Runnable() {
				
			        public void run() {
			            enablelockapps();
			        }
				
				},150);
				
			} else if (resultCode == -302) {
			}
		} 
			if (resultCode == FingerPrintManager.MSG_RES.RES_ADD_CODE) {
				updateListView();
				PreferenceUtils.setSettingFpScreenLockOffOn(true);
				
			} else if (resultCode == -302){
			}
		}

	/*
	 * 通过游标，初始化FingerPrintModel列表
	 */
	private List<FingerPrintModel> getFingerPrintFromCursor(Cursor cursor) {
		Log.e(TAG, "getFingerPrintFromCursor...begin");
		List<FingerPrintModel> fingerList = new ArrayList<FingerPrintModel>();
		while (cursor.moveToNext()) {
			FingerPrintModel finger = new FingerPrintModel();
			finger.fp_id = cursor
					.getInt(cursor.getColumnIndex(FpsTable.COL_ID));
			finger.fp_name = cursor.getString(cursor
					.getColumnIndex(FpsTable.COL_FP_NAME));
			finger.fp_data_index = cursor.getInt(cursor
					.getColumnIndex(FpsTable.COL_FP_DATA_INDEX));
			finger.fp_data_path = cursor.getInt(cursor
					.getColumnIndex(FpsTable.COL_FP_DATA_PATH));
			finger.fp_lunch_package = cursor.getString(cursor.getColumnIndex(FpsTable.COL_FP_LUNCH_PACKAGE));
			fingerList.add(finger);
		}
		Log.e(TAG, "getFingerPrintFromCursor success...");
		return fingerList;
	}
	
	
	
		/**
		 * 通过得到coursor更新listview
		 */
	private void updateListView() {
		Log.e(TAG, "---updateListView---begin");
		
		mFingerPrintModels.clear();//清除	
		Cursor mCursor = queryAllFinger();
		
		mCursorAdapter = new SimpleCursorAdapter(this, R.layout.fp_item_list, mCursor,
						new String[] { FpsTable.COL_FP_NAME},
						new int[] { R.id.text1 });
		mFingerPrintModels = getFingerPrintFromCursor(mCursor);
        mListView.setAdapter(mCursorAdapter);
        mListView.setOnItemClickListener(mOnItemClickListener);
        int fpSize = mFingerPrintModels.size();
        Log.e(TAG, "mFingerPrintModels.size()" + fpSize);

		// 如果指纹个数为0，要禁用指纹的相关解锁
        if (fpSize == 0) {
            Settings.System.putString(mContext.getContentResolver(), "com_btlfinger_fingerprint_lastapp_package", PackagesConstant.FINGERPRINTUNLCOK_PACKAGENAME);
            PreferenceUtils.setSettingFpAppLockOffOn(false);
            PreferenceUtils.setSettingFpScreenLockOffOn(false);
          //设置包裹listeView的线性布局不可见
            mLinearLayout.setVisibility(View.GONE);
        }else{
        	//设置包裹listeView的线性布局可见
        	mLinearLayout.setVisibility(View.VISIBLE);
        }
        /**
         * 如果条目大于5就变灰
         */
        ((TextView)findViewById(R.id.id_manage_addfp_tv)).setEnabled(mFingerPrintModels.size() < 5 ? true : false);

        if (fpSize < 5) {
            mAddFpTv.setTextColor(Color.parseColor("#000000"));
        } else {
            mAddFpTv.setTextColor(Color.GRAY);
        }
        Log.e(TAG, "---updateListView---end");
	}
	/**
	 * 查询所有指纹
	 * @return
	 */
	private Cursor queryAllFinger() {
		Log.e(TAG, "queryAllFinger...begin");
		Uri FPURI = Uri.parse("content://com.btlfinger.fingerprint.provider/fp_data_table");
		Cursor cursor = this.getContentResolver().query(FPURI, null, null, null, null);
		Log.i(TAG, "queryAllFinger...end; count = " + cursor.getCount());
        return cursor;
	}
	/**
	 *设置条目监听
	 */
	private OnItemClickListener mOnItemClickListener = new OnItemClickListener() {
		public void onItemClick(android.widget.AdapterView<?> parent,
				View view, int position, long id) {
				Log.i(TAG, "position " + position);
				if (mFingerPrintModels != null)
			    startDetailActivity(mFingerPrintModels.get(position));
				
		};
	};
	
	/**
	 * 指纹启动模式
	 * @param model
	 */
	private void startDetailActivity(FingerPrintModel model) {
		Intent intent = new Intent();
		Bundle bundle = new Bundle();
		bundle.putParcelable(FingerPrintManager.INTENT_KEY.KEY_PARCEL_FP, model);
		intent.putExtras(bundle);
		intent.setClass(this, FpDetail.class);
		//PreferenceUtils.enablePwd(true);
		mStopFlag = false;
		startActivityForResult(intent, FingerPrintManager.MSG_RES.REQ_DETAIL_CODE);
	}
	 /**
     * 开启EnableLockAppActivity界面
     * @param view
     */
    public void gotoLockApps(View view) {
    	Intent intent = new Intent(this, EnableLockAppActivity.class);
    	mStopFlag = false;
    	startActivity(intent);
    }
	/**
	 * 添加手指
	 * @param view
	 */
	public void addFP(View view) {
		int fpSize = mFingerPrintModels.size();
		Log.i(TAG, "addFP... fpSize:" + fpSize);
		if (fpSize < 5) {
			addFingerPrint(view.getId());
		}
	}
	public void addFingerPrint(int requestCode) {
	    //PreferenceUtils.enablePwd(true);
		Intent intent = new Intent();
		intent.setClass(this, AddFpActivity.class);
		mStopFlag = false;
		startActivityForResult(intent, 301);
	}
	
	public void setFingerunlock(View view) {
		boolean isChecked = !mToggleButton.isChecked();
		mToggleButton.setChecked(isChecked);

		if(isChecked)
		{
			if (mFingerPrintModels != null && mFingerPrintModels.size() == 0) {
				addFingerPrint(view.getId());
			}
		}

		PreferenceUtils.setSettingFpScreenLockOffOn(isChecked);
		Log.i(TAG, "fingerprintCheckedChangeListener" + "mToggleButton.isChecked" + isChecked
				+ "setSettingFpScreenLockOffOn" +PreferenceUtils.isSettingFpScreenLockOn());
	}

	public void enablelockapps() {
		//PreferenceUtils.enablePwd(true);
		Intent intent = new Intent();
		intent.setClass(this, EnableLockAppActivity.class);
		startActivityForResult(intent,111);
	}
	
	/**
	 * 设置title返回home页面
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
