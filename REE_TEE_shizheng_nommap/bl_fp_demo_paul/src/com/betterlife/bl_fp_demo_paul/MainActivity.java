package com.betterlife.bl_fp_demo_paul;

import com.betterlife.fingerprint.CaptureView;
import com.betterlife.fingerprint.FingerprintData;
import com.betterlife.fingerprint.FpNative;

import android.app.Activity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

public class MainActivity extends Activity {
	private static String TAG = "sjx";
	public static int sDacp = 0x8c;// 0x8c;
	public static int sGain_0x31 = 1;
	public static int sGain_0x32 = 1;
	public static int[] result = new int[10];
	public static int sSaved = 0;
	public static int sModule = 0;
	public static int sScale = 1;
	public static int fileCounts = 1;
	Button mbtnCapture;
	TextView mDacpTextVew;
	SeekBar mDacpSeekBar;
	byte mdatabuf[];
	int mparams[];
	int mresult[];
	boolean isCapture = false;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		Log.d(TAG, "width:"+FingerprintData.getInstance().width
				+ "height:" + FingerprintData.getInstance().height);
		//sDacp = FingerprintData.getInstance().capdacp;
		mDacpTextVew = (TextView) findViewById(R.id.dacp_text);
		mDacpSeekBar = (SeekBar) findViewById(R.id.seekBar_dacp);
		mDacpSeekBar.setMax(255);
		mDacpSeekBar.setProgress(sDacp);
		mDacpTextVew.setText(Integer.toString(sDacp));
		mdatabuf = new byte[128 * 128];
		mparams = new int[10];
		mresult = new int[10];
		
		//seekbar listener
		mDacpSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			
			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				// TODO Auto-generated method stub
				mDacpTextVew.setText(Integer.toString(progress));
				sDacp = progress;
			}
		});
		
		//dacp test change listerner
		mDacpTextVew.addTextChangedListener(new TextWatcher() {
			
			@Override
			public void onTextChanged(CharSequence s, int start, int before,
					int count) {
				// TODO Auto-generated method stub
				int progress = 0;
				progress = Integer.parseInt(s.toString());
				mDacpSeekBar.setProgress(progress);
				sDacp = progress;
			}
			
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count,
					int after) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void afterTextChanged(Editable s) {
				// TODO Auto-generated method stub
				
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
		FpNative.FpUninit();
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
