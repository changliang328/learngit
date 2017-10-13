package com.btlfinger.fingerprint.utils;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.Thread.UncaughtExceptionHandler;
import java.util.Date;

import android.content.Context;
import android.os.Environment;
import android.util.Log;
/**
 * 异常截取工具类
 *
 * @author blestech
 * @since 2015-11-26
 */

public class CrashHandlerUtils implements UncaughtExceptionHandler {

	/**
	 * 全局处理异常
	 */
	private static CrashHandlerUtils instance;
	private Context mContext;  
	private Thread.UncaughtExceptionHandler mDefaultHandler;  
	private String TAG = "CrashHandlerUtils";

	/**
	 * 获取CrashHandler实例，单例模式
	 * 
	 * @return 返回CrashHandler实例
	 */

	public static CrashHandlerUtils getInstance() {

		if (instance == null) {
			instance = new CrashHandlerUtils();
		}
		return instance;

	}
	/**
	 * 初始化
	 * @param ctx
	 */
	public void init(Context ctx) {
		mContext = ctx;
		mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();
		Thread.setDefaultUncaughtExceptionHandler(this);
	}

	/**
	 * 获取造成crash异常的具体文件名、类名、方法名、行号
	 */
	public void uncaughtException(Thread arg0, Throwable arg1) {
		Log.e(TAG, "CrashHandlerUtils -->uncaughtException");
		String logdir = null;
		if (Environment.getExternalStorageDirectory() != null) {
			logdir = Environment.getExternalStorageDirectory()
					.getAbsolutePath()
					+ File.separator
					+ "snda"
					+ File.separator + "log";

			File file = new File(logdir);
			boolean mkSuccess;
			if (!file.isDirectory()) {
				mkSuccess = file.mkdirs();
			}
		}
		try {
			FileWriter fw = new FileWriter(logdir + File.separator
					+ "error.log", true);
			fw.write(new Date() + "\n");
			StackTraceElement[] stackTrace = arg1.getStackTrace();
			fw.write(arg1.getMessage() + "\n");
			for (int i = 0; i < stackTrace.length; i++) {
				fw.write("file:" + stackTrace[i].getFileName() + "class:"
						+ stackTrace[i].getClassName() + "method:"
						+ stackTrace[i].getMethodName() + "line:"
						+ stackTrace[i].getLineNumber() + "\n");
			}
			fw.write("\n");
			fw.close();
		} catch (IOException e) {
			Log.e("crash handler", "load file failed...", e.getCause());
		}
		arg1.printStackTrace();
		android.os.Process.killProcess(android.os.Process.myPid());
	}
}
