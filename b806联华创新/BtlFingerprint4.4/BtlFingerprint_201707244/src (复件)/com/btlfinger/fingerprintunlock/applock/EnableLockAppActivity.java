package com.btlfinger.fingerprintunlock.applock;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import android.app.ActionBar;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

import com.btlfinger.fingerprintunlock.AppData;
import com.btlfinger.fingerprintunlock.PackagesConstant;
import com.btlfinger.fingerprintunlock.R;
import com.btlfinger.fingerprintunlock.support.PreferenceUtils;
/**
 * 应用锁界面
 * 
 * @author blestech
 * @since 2015-11-26
 */

public class EnableLockAppActivity extends Activity {

	private static final String TAG = "EnableLockAppActivity";
	private static CommonAdapter<AppInfo> mAdapter;
	private static ListView mAppsListView;
	private List<AppInfo> mAppsData = null;
	private PackageManager mPackageManager;

	private static ProgressBar mProgressBar = null;
	private LoadAppsThread mLoadAppsThread = null;
	private ToggleButton mToggleButton = null;
	private boolean mStopFlag = true;

	private static Context mContext = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mContext = this;

		setContentView(R.layout.activity_enablelock_apps);
		mAppsData = new ArrayList<AppInfo>();
		ActionBar actionBar = getActionBar();
		actionBar.setHomeButtonEnabled(false);
		actionBar.setDisplayShowTitleEnabled(true); // 可以显示标题栏
		actionBar.setDisplayShowHomeEnabled(false);// actionBar左侧图标是否显示
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setDisplayUseLogoEnabled(false);

		String packagenameString = getPackageName();

		mToggleButton = (ToggleButton) findViewById(R.id.applock_off_on_tb);
		//mToggleButton.setOnCheckedChangeListener(appLockCheckedChangeListener);	
		
		//mProgressBar = (ProgressBar) findViewById(R.id.id_enablelock_apps_pb);
		mPackageManager = getPackageManager();

		mAppsListView = (ListView) findViewById(R.id.id_lv_main);

		mAdapter = new CommonAdapter<AppInfo>(EnableLockAppActivity.this,
				mAppsData, R.layout.lockapp_item_list) {

			@Override
			public void convert(ViewHolder holder, final AppInfo item) {
				holder.setText(R.id.id_item_appname_tv, item.appName);
				holder.setImageDrawable(R.id.id_item_appicon_iv, item.appIcon);

				final ToggleButton tb = holder.getView(R.id.id_item_needlock_tb);
				tb.setChecked(item.isChecked());
				tb.setEnabled(PreferenceUtils.isSettingFpAppLockOn());
				
				item.mHolder = holder;
				item.mHolder.mToggleButton.setEnabled(mToggleButton.isChecked() == true?true:false);
				tb.setOnClickListener(new OnClickListener() {

					@Override
					public void onClick(View v) {
						item.setChecked(tb.isChecked());

						/*String lastPkgName = Settings.System
								.getString(
										AppData.getContext()
												.getContentResolver(),
										PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME);

						if (lastPkgName.equals(item.packageName)) {
							if (!tb.isChecked()) {
								Settings.System
										.putString(
												AppData.getContext()
														.getContentResolver(),
												PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME,
												"");
							} else {
								Settings.System
										.putString(
												AppData.getContext()
														.getContentResolver(),
												PackagesConstant.SETTINGS_LAST_LOCK_APP_PACKAGENAME,
												item.packageName);
							}
						}*/
					}
				});

				RelativeLayout container = (RelativeLayout) holder
						.getView(R.id.id_app_item_container);
				container.setOnClickListener(new OnClickListener() {

					@Override
					public void onClick(View v) {
						if (mToggleButton.isChecked()) {
							tb.setChecked(!item.isChecked());
							item.setChecked(tb.isChecked());
						}

					}
				});
			}
		};

		mLoadAppsThread = new LoadAppsThread();
		mLoadAppsThread.start();
		
		mToggleButton.setChecked(PreferenceUtils.isSettingFpAppLockOn());
		/**
		 * mAppsListView透明度
		 */
		this.mAppsListView.setAlpha(mToggleButton.isChecked() == true ?(float)1:(float)0.6);

		LinearLayout containers = (LinearLayout)findViewById(R.id.id_enablelock_apps_sw_ll);
		containers.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				Log.e(TAG,
						"发生变化 ： containers.setOnClickListener"
								+ mToggleButton.isChecked());
				
				boolean isChecked = !mToggleButton.isChecked();
				mToggleButton.setChecked(isChecked);
				PreferenceUtils.setSettingFpAppLockOffOn(isChecked);
				mAdapter.notifyDataSetChanged();
				
				/**
				 * 设置mAppsListView透明度
				 */
				if (isChecked) {				
						mAppsListView.setAlpha((float)1);
				}else{
					mAppsListView.setAlpha((float)0.6);
				}
			}
		});	
	}
	
	private OnCheckedChangeListener appLockCheckedChangeListener = new OnCheckedChangeListener() {

		@Override
		public void onCheckedChanged(CompoundButton buttonView,
				boolean isChecked) {
			Log.e(TAG,
					"发生变化 ： appLockCheckedChangeListener"
							+ mToggleButton.isChecked());
			PreferenceUtils.setSettingFpAppLockOffOn(mToggleButton.isChecked());
			mAdapter.notifyDataSetChanged();
			/**
			 * 设置mAppsListView透明度
			 */
			if(isChecked == true)
			{
				mAppsListView.setAlpha((float)1);
			}
			else
			{
				mAppsListView.setAlpha((float)0.6);
			}
			
		}
	};

	private class LoadAppsThread extends Thread {
		@Override
		public void run() {
			queryAppInfo();
		}
	}

	private boolean mFlag = false;
	private Handler mHandler = new Handler() {
		public void handleMessage(android.os.Message msg) {

			if (msg.what == 0) {
				//mProgressBar.setVisibility(View.GONE);
				mAppsListView.setAdapter(mAdapter);
				mAdapter.notifyDataSetChanged();
			}
		}
	};
	
	// 获得所有启动Activity的信息，类似于Launch界面
	public void queryAppInfo() {
		String[] exclueApps = getResources()
				.getStringArray(R.array.exclude_app);// 需要过滤掉的应用

		List<String> existLockAppsList = new ArrayList<String>();
		List<PackageInfo> allPackages = getPackageManager()
				.getInstalledPackages(0);

		StringBuilder stringBuilder = new StringBuilder();
		String appString = Settings.System.getString(getContentResolver(),
				PackagesConstant.SETTINGS_NEEDLOCK_APP_PACKAGENAMES);
		
		if (appString != null) {
			String[] appsStrings = appString.split("\\|");
			existLockAppsList = Arrays.asList(appsStrings);
		}

		PackageManager pm = this.getPackageManager(); // 获得PackageManager对象
		Intent mainIntent = new Intent(Intent.ACTION_MAIN, null);
		mainIntent.addCategory(Intent.CATEGORY_LAUNCHER);
		// 通过查询，获得所有ResolveInfo对象.
		List<ResolveInfo> resolveInfos = pm
				.queryIntentActivities(mainIntent, 0);
		Collections.sort(resolveInfos,
				new ResolveInfo.DisplayNameComparator(pm));
		if (mAppsData != null) {
			mAppsData.clear();
			for (ResolveInfo reInfo : resolveInfos) {
				String activityName = reInfo.activityInfo.name; // 获得该应用程序的启动Activity的name
				String pkgName = reInfo.activityInfo.packageName; // 获得应用程序的包名
				String appLabel = (String) reInfo.loadLabel(pm); // 获得应用程序的Label
				Drawable icon = reInfo.loadIcon(pm); // 获得应用程序图标

				android.util.Log.i(TAG, "pkgName = " + pkgName);

				// 创建一个AppInfo对象，并赋值
				AppInfo appInfo = new AppInfo();
				appInfo.packageName = pkgName;
				appInfo.appName = appLabel;
				appInfo.appIcon = (icon);

				if (existLockAppsList.contains(pkgName)) {
					appInfo.setChecked(true);

					android.util.Log.i(TAG, "existLockAppsList 1");
				}
    			boolean fExcluedApp = false;
				for(int  i = 0; i< exclueApps.length; i++){
					if(pkgName.compareTo(exclueApps[i])==0){
					   fExcluedApp = true;
					   break;
					}
				}
    			
				/*
				 * com.xiami.walkman"音乐要不要过滤呢？ 屏蔽一键清理、语音助手
				 */
				if (!pkgName
						.equals(PackagesConstant.FINGERPRINTUNLCOK_PACKAGENAME) && !fExcluedApp) {
					int mSamePkgName = 0;
					for (int i = 0; i < mAppsData.size(); i++) {
						if (mAppsData.get(i).packageName.equals(pkgName)) {// 过滤相同包名
							mSamePkgName += 1;
							break;
						}
					}
					if (mSamePkgName == 0) {
						mAppsData.add(appInfo); // 添加至列表中
					}
				}
			}
		}

		mFlag = true;
		mHandler.sendEmptyMessage(0);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	@Override
	protected void onStop() {
		super.onStop();

		new SaveLockAppsThread().start();

		if(mStopFlag){
			finish();
		}
	}
	
  @Override
    protected void onResume() {
        Log.e(TAG, "onResume");
        super.onResume();
        mStopFlag = true;
  }

	private class SaveLockAppsThread extends Thread {
		@Override
		public void run() {
			if (mFlag) {
				if (mAppsData != null) {
					Settings.System
							.putString(
									mContext.getContentResolver(),
									PackagesConstant.SETTINGS_NEEDLOCK_APP_PACKAGENAMES,
									null);
					StringBuilder stringBuilder = new StringBuilder();
					for (int i = 0; i < mAppsData.size(); i++) {
						if (mAppsData.get(i).isChecked) {
							stringBuilder.append(mAppsData.get(i).packageName
									+ "|");
							Log.v(TAG, "tmpInfo : " + stringBuilder.toString());
							Settings.System
									.putString(
											mContext.getContentResolver(),
											PackagesConstant.SETTINGS_NEEDLOCK_APP_PACKAGENAMES,
											stringBuilder.toString());
						}
					}
				}
			}
		}
	}
	/**
	 * App信息
	 * @author blestech
	 *
	 */
	class AppInfo {
		public String appName = "";
		public String packageName = "";
		public String versionName = "";
		public int versionCode = 0;
		public Drawable appIcon = null;
		private boolean isChecked = false;
		public ViewHolder mHolder;

		public boolean isChecked() {
			return isChecked;
		}

		public void setChecked(boolean check) {
			isChecked = check;
		}

		public void print() {
			Log.v(TAG, "Name:" + appName + " |  Package:" + packageName);
		}

	}
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
