package com.betterlife.bl_fp_demo_paul;

import com.betterlife.fingerprint.CaptureView;
import com.betterlife.fingerprint.FpNative;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MainActivity extends Activity {
	FpNative mfpNative;
	Button mbtnCapture;
	byte mdatabuf[];
	int mparams[];
	int mresult[];
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		mfpNative = new FpNative();
		mfpNative.FpInit();
		mbtnCapture = (Button) findViewById(R.id.capture);
		mdatabuf = new byte[128 * 128];
		mparams = new int[10];
		mresult = new int[10];
		
		
		mbtnCapture.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				//while(true)
				{
					mfpNative.FpGetFingerRawImage(mdatabuf, mparams, mresult);
				}
			}
		});
	}
	@Override
	protected void onResume() {
		CaptureView.bStop = false;
		CaptureView.bExit = false;
		super.onResume();
	}

	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		CaptureView.bStop = true;
		CaptureView.bExit = true;
		super.onPause();

	}
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		mfpNative.FpUninit();
		super.onDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
