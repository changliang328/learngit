package com.btlfinger.fingerprintunlock.ui.support;

import com.btlfinger.fingerprintunlock.R;

import android.app.ActionBar;
import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.TextView;
import android.content.Intent;



/**
 * 密码选择界面
 * 
 * @author blestech
 * @since 2015-12-26
 */
public class PasswdActivity extends Activity{

    private static final String TAG = "PasswdActivity";
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_passwd);

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
	}

	 public void gotoManageF(View view) {
	 	  //Log.d(TAG,"gotoManageF");
          Intent intent = new Intent(this, ManagerPINActivity.class);
          startActivity(intent);
		  finish();
    }

}
