package com.btlfinger.fingerprintunlock.ui.support;

import com.btlfinger.fingerprint.utils.AppInfoUtils;
import com.btlfinger.fingerprintunlock.R;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
/**
 * 指纹版本界面
 *
 * @author blestech
 * @since 2015-11-26
 */
public class ManageFpVersion extends Activity {
	private Button mVersionButton;
	private static Context mContext = null;
	private TextView application_version_tv;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.actvity_manage_version);
		ActionBar actionBar = getActionBar();
		actionBar.setTitle(R.string.app_name);
		actionBar.setHomeButtonEnabled(false);
		actionBar.setDisplayShowTitleEnabled(true);
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setDisplayUseLogoEnabled(false);
		actionBar.setDisplayShowHomeEnabled(false);
		mContext = this;

//		mVersionButton = (Button) findViewById(R.id.manage_version_bt);
		application_version_tv = (TextView) findViewById(R.id.application_version_tv);
		application_version_tv.setText(AppInfoUtils.getAppVersionName(this));
	}

	/**
	 * 开启ManageFpActivity
	 * 
	 * @param view
	 */
	public void gotoManageP(View view) {
		Intent intent = new Intent(this, ManageFpActivity.class);
		startActivity(intent);
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
