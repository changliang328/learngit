package com.btlfinger.fingerprintunlock.ui.support;

import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.fingerprint.FingerPrintManager;
import com.btlfinger.fingerprint.FingerPrintModel;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.text.TextUtils;
import android.widget.Toast;

/**
 * 指纹详情
 * 
 * @author blestech
 * @since 2015-11-26
 */
public class FpDetail extends Activity implements OnClickListener {
	private String TAG = "FpDetail";
	private FingerPrintModel mFingerPrintModel = null;
	private TextView mTvFpName = null;
	private EditText mEdtFpNewName = null;
	private Button mBtnMatchApp;
	private Button mBtnRename = null;
	private Button mBtnConfirmRename = null;
	private Button mBtnDelete = null;
	private FingerPrintManager mFingerPrintManager = null;


	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_fp_detail);

		ActionBar actionBar = getActionBar();
		actionBar.setTitle(R.string.app_name);
		actionBar.setHomeButtonEnabled(false);
		actionBar.setDisplayShowTitleEnabled(true); // 可以显示标题
		actionBar.setDisplayShowHomeEnabled(false);// actionBar左侧图标是否显示
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setDisplayUseLogoEnabled(false);

		mFingerPrintModel = (FingerPrintModel) getIntent().getExtras().get(
				FingerPrintManager.INTENT_KEY.KEY_PARCEL_FP);
		mFingerPrintManager = new FingerPrintManager(this);

		mEdtFpNewName = (EditText) findViewById(R.id.id_edt_fp_rename);
		mBtnMatchApp = (Button) findViewById(R.id.id_btn_match_app);
		mBtnConfirmRename = (Button) findViewById(R.id.id_btn_confirm_rename);
		mTvFpName = (TextView) findViewById(R.id.id_tv_fp_rename);
		mBtnRename = (Button) findViewById(R.id.id_btn_rename);
		mBtnDelete = (Button) findViewById(R.id.id_btn_delete);

		mTvFpName.setText(mFingerPrintModel.fp_name);
		mBtnRename.setOnClickListener(this);
		mBtnDelete.setOnClickListener(this);
		mBtnConfirmRename.setOnClickListener(this);
		mBtnMatchApp.setOnClickListener(this);
		
		
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.id_btn_rename:
			mBtnConfirmRename.setVisibility(View.VISIBLE);
			mEdtFpNewName.setVisibility(View.VISIBLE);
			mEdtFpNewName.setText(mTvFpName.getText().toString());
			mTvFpName.setVisibility(View.GONE);
			mBtnRename.setVisibility(View.GONE);
			mEdtFpNewName.requestFocus();//get the focus
			break;

		case R.id.id_btn_delete:
			createDeleteDialog();
			break;
			
		case R.id.id_btn_match_app:
			
			Thread.currentThread().getId();
			Intent intent = new Intent();
			String oldpackage = mFingerPrintModel.fp_lunch_package;
			if( oldpackage != null )
				intent.putExtra("lunchpackagename", oldpackage); 
			else
			intent.putExtra("lunchpackagename", ""); 
			intent.putExtra ("fingermodelindex", mFingerPrintModel.fp_data_index); 
			
			Bundle bundle = new Bundle();
			bundle.putParcelable(FingerPrintManager.INTENT_KEY.KEY_PARCEL_FP, mFingerPrintModel);
			intent.putExtras(bundle);
			Log.i(TAG, "onActivityResult: " + mFingerPrintModel);
			Log.i(TAG, "onActivityResult: " + mFingerPrintManager);
			intent.setClass(this, SelectAppActivity.class);
			startActivityForResult(intent, 0);
			break;

		case R.id.id_btn_confirm_rename:
			//PreferenceUtils.enablePwd(false);
			/* 判断输入的重命名是否为空 */
			String newName = mEdtFpNewName.getText().toString().trim();
			if (!TextUtils.isEmpty(newName)) {
				Log.i(TAG, "index = " + mFingerPrintModel.fp_data_index);
				Log.i(TAG, "name = " + mEdtFpNewName.getText().toString());
				mFingerPrintManager.updateFpNameByIndex(mEdtFpNewName.getText()
						.toString(), mFingerPrintModel.fp_data_index + "");
				finish();
			} else {
				showToast();
			}

			break;
		default:
			break;
		}
	}

	Toast mToast = null;

	private void showToast() {
		if (mToast == null) {
			mToast = Toast.makeText(FpDetail.this,
					getString(R.string.new_name_warning), Toast.LENGTH_SHORT);
		}
		mToast.show();
	}
	/**
	 * 创建删除指纹对话框
	 */
	public void createDeleteDialog() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.fp_detail_delete)
				.setPositiveButton(R.string.confirm, confirmListener)
				.setNegativeButton(R.string.cancel, null).create().show();
	}

	private DialogInterface.OnClickListener confirmListener = new DialogInterface.OnClickListener() {
		@Override
		public void onClick(DialogInterface dialog, int which) {
			//PreferenceUtils.enablePwd(false);
			mFingerPrintManager.deleteFp(mFingerPrintModel.fp_data_index + "");
			finish();
			Log.e(TAG, "Index : " + mFingerPrintModel.fp_data_index);
		}
	};

	protected void onStop() {
		super.onStop();
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem menuItem) {
		switch (menuItem.getItemId()) {

		case android.R.id.home:
			setResult(111);
			//PreferenceUtils.enablePwd(false);
			finish();
			break;
		default:
			break;
		}

		return true;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			if (mBtnConfirmRename.getVisibility() == View.GONE) {
				setResult(111);
				//PreferenceUtils.enablePwd(false);
				return super.onKeyDown(keyCode, event);
			} else {
				mTvFpName.setVisibility(View.VISIBLE);
				mBtnRename.setVisibility(View.VISIBLE);
				mBtnConfirmRename.setVisibility(View.GONE);
				mEdtFpNewName.setVisibility(View.GONE);
				return true;
			}

		}
		return super.onKeyDown(keyCode, event);
	}
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
		Log.i(TAG, "onActivityResult:called ");

		try{
			if(data.getExtras().containsKey("packagename")){
				if(resultCode==RESULT_OK)
				{
					String PackageName = data.getStringExtra("packagename");
					
					Log.i(TAG, "onActivityResult: " + PackageName);
					mFingerPrintModel.fp_lunch_package = PackageName;
				}
			}
		}catch(Exception e)
		{
		}
		super.onActivityResult(requestCode, resultCode, data);
	}
}
