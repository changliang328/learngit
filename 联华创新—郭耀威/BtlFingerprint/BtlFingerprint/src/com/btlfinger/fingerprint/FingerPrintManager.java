package com.btlfinger.fingerprint;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.btlfinger.fingerprint.FingerPrintModel;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.os.Parcelable;
import android.util.Log;
import com.btlfinger.fingerprint.dao.DbHelper;
import com.btlfinger.fingerprint.dao.FpsTable;
import android.provider.Settings;
/**
 * Framework
 * 
 * @author blestech
 * @since 2015-11-26
 */
public class FingerPrintManager {

	public static class INTENT_KEY {

		public static final String KEY_FP_FILE_INDEX = "key_fp_file_index";

		public static final String KEY_PARCEL_FP = "key_parcel_fp";
	}
	
	public static class MSG_RES {

		public static final int MSG_REG_START = 101;
		public static final int MSG_REG_STOP = 102;

		public static final int RES_HARDWARE_ERROT = -3;

		public static final int RES_OVERTIME = -1;

		public static final int RES_NOTIN_CENTER = -2;

		public static final int RES_TOO_QUICK = -100;

		public static final int RES_EMPTY = -124;

		public static final int OPEN_FILE_FAIL = -10;

		public static final int MSG_REG_CANCEL = -2047;

		public static final int MSG_REG_FAIL = -2046;
		
		public static final int MSG_REG_OVERTIME = -2015;
		
		public static final int MSG_REG_OK = 0;

		public static final int REQ_ADD_CODE = 301;
		public static final int RES_ADD_CODE = 302;

		public static final int REQ_DETAIL_CODE = 303;
		public static final int RES_RENAME_CODE = 304;
		public static final int RES_DELETE_CODE = 305;
    }
	public static class Params{
		public static final int PARAM_GET_FP = 400;
		public static final int PARAM_GET_FP_FILE_COUNT = 401;
		public static final int PARAM_GET_FP_COUNT = 402;
		public static final int PARAM_ADD_FP = 403;
		public static final int PARAM_DELETE_FP = 404;
		public static final int PARAM_RENAME_FP = 405;
		public static final int PARAM_MATCH_FP = 406;
		public static final int PARAM_WAITSCREEN_ON = 407;
		public static final int PARAM_SHUTDOWN = 500;
	}

    private static String TAG = "FingerPrintManager";
    public static final String SETTINGS_KEY_USEDTO_SCREENLOCK = "com_btlfinger_fingerprint_usedto_screenlock";
    public static final String SETTINGS_KEY_USEDTO_APPLOCK = "com_btlfinger_fingerprint_usedto_applock";
    public static String SETTINGS_NEEDLOCK_APP_PACKAGENAMES = "com_btlfinger_fingerprint_needlockapp_package";
    public static String SETTINGS_LAST_LOCK_APP_PACKAGENAME = "com_btlfinger_fingerprint_lastapp_package";
    public static String SETTINGS_INITIALIZE_PWD =  "com_btlfinger_fingerprint_initialize_pwd";

	public static final String FP_DATA_DIR = "/data/data/com.btlfinger.fingerprintunlock/fpfile/";

	private static DbHelper mDbHelper = null;
	//public static String SETTINGS_NEEDLOCK_APP_PACKAGENAMES;
	private Context mContext = null;

	public FingerPrintManager(Context context) {
        init(context);
	}

	public void init(Context context){
		mContext = context;
		mDbHelper = new DbHelper(context);
	}

    public List<FingerPrintModel> getFingerPrintFromCursor(Cursor cursor) {
        List<FingerPrintModel> fingerList = new ArrayList<FingerPrintModel>();
        while (cursor.moveToNext()) {
            FingerPrintModel finger = new FingerPrintModel();
            finger.fp_id = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_ID));
            finger.fp_name = cursor.getString(cursor.getColumnIndex(FpsTable.COL_FP_NAME));
            finger.fp_data_index = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_FP_DATA_INDEX));
            finger.fp_data_path = cursor.getInt(cursor.getColumnIndex(FpsTable.COL_FP_DATA_PATH));
            finger.fp_lunch_package = cursor.getString(cursor.getColumnIndex(FpsTable.COL_FP_LUNCH_PACKAGE));
            Log.i("FingerPrintManager", "fp_lunch_package:"+finger.fp_lunch_package);
            Log.i("FingerPrintManager", "fp_id:"+finger.fp_id);
            Log.i("FingerPrintManager", "fp_name:"+finger.fp_name);
            Log.i("FingerPrintManager", "fp_data_index:"+finger.fp_data_index);
            Log.i("FingerPrintManager", "fp_data_path:"+finger.fp_data_path);
            fingerList.add(finger);
        }
        Log.e(TAG, "getFingerPrintFromCursor success...");
        return fingerList;
    }

	public void deleteFp(String index) {
        Log.e(TAG + "delete","deleteFp index: "+index);
        List<String> filepaths = mDbHelper.getFpFilePathByIndex(index);
        for (int i = 0; i < filepaths.size(); i++) {
		    Log.e(TAG + "delete","deleteFp fileName: "+filepaths.get(i));
			File file = new File(filepaths.get(i));
			if (file.exists()) {
				file.delete();
			}
		}
		mDbHelper.deleteFpByIndex(index);
	}

	public void updateFpNameByIndex(String fp_new_name, String fp_index_string) {
        mDbHelper.updateFpNameByIndex(fp_new_name, fp_index_string);
    }
    
	public void updateFpLunchByIndex(String fp_lunch_package, String fp_index_string) {
        mDbHelper.updateFpLunchByIndex(fp_lunch_package, fp_index_string);
    }

	public static boolean init_folder() {
		boolean create_data_folder = false;

		File datafiles = new File(FP_DATA_DIR);
		if (!datafiles.exists()) {
			create_data_folder = datafiles.mkdir();
		} else {
			create_data_folder = true;
		}

		return create_data_folder;
	}

	public String[] getAllFpData() {
		String[] allfiles = null;
		File datafiles = new File(FP_DATA_DIR);
		if (init_folder() == true) {
			allfiles = datafiles.list();
		} else {
			datafiles.mkdir();
		}
		return allfiles;
	}

    public int[] getFileIndex(){
        List<Integer> indexList = mDbHelper.getAllFileIndex();
        int[] index = new int[indexList.size()];
        for (int i = 0; i < indexList.size(); i++) {
            Log.e(TAG, "File-index["+i+"] = " + indexList.get(i));
            index[i] = indexList.get(i);
        }
        return index;
    }
    
    public int[] getAllTemplateLength(){
        List<Integer> templateLength = mDbHelper.getAllTemplateLength();
        int[] index = new int[templateLength.size()];
        for (int i = 0; i < templateLength.size(); i++) {
            Log.e(TAG, "Template-index["+i+"] = " + templateLength.get(i));
            index[i] = templateLength.get(i);
        }
        return index;
    }
	
    public int getMaxIndex(){
	    return mDbHelper.getCurrentMaxFpIndex();
    }

	public int getMaxId(){
		return mDbHelper.getCurrentMaxFpId();
	}
	
	public int getNewFpNameIndex(){
		return mDbHelper.getNewFpNameIndex();
	}

}
